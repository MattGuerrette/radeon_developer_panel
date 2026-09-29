// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the QML-facing wrapper around a trace source view model.

#include "model/capture_view_model.h"

#include <algorithm>
#include <utility>

#include <QDir>
#include <QFileInfo>
#include <QLocale>

CaptureFileListModel::CaptureFileListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

CaptureFileListModel::~CaptureFileListModel() = default;

void CaptureFileListModel::Refresh(const QString& directory, const QString& extension)
{
    std::vector<Entry> entries;

    if (!directory.isEmpty())
    {
        const QDir dir(directory);
        const QFileInfoList files = dir.entryInfoList({QString("*.%1").arg(extension)}, QDir::Files, QDir::Time);

        entries.reserve(static_cast<size_t>(files.size()));
        for (const QFileInfo& info : files)
        {
            entries.push_back({info.fileName(), info.absoluteFilePath(), QLocale().formattedDataSize(info.size()), info.lastModified()});
        }
    }

    beginResetModel();
    entries_ = std::move(entries);
    endResetModel();
}

QString CaptureFileListModel::PathAt(const int row) const
{
    if (row < 0 || row >= static_cast<int>(entries_.size()))
    {
        return {};
    }

    return entries_[static_cast<size_t>(row)].path;
}

void CaptureFileListModel::RemoveRow(const int row)
{
    if (row < 0 || row >= static_cast<int>(entries_.size()))
    {
        return;
    }

    beginRemoveRows({}, row, row);
    entries_.erase(entries_.begin() + row);
    endRemoveRows();
}

int CaptureFileListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(entries_.size());
}

QVariant CaptureFileListModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(entries_.size()))
    {
        return {};
    }

    const Entry& entry = entries_[static_cast<size_t>(index.row())];

    switch (role)
    {
    case Qt::DisplayRole:
    case kNameRole:
        return entry.name;
    case kSizeRole:
        return entry.size;
    case kModifiedRole:
        return entry.modified.toString("yyyy-MM-dd HH:mm");
    case kPathRole:
        return entry.path;
    default:
        return {};
    }
}

QHash<int, QByteArray> CaptureFileListModel::roleNames() const
{
    return {{kNameRole, "name"}, {kSizeRole, "size"}, {kModifiedRole, "modified"}, {kPathRole, "path"}};
}

CaptureViewModel::CaptureViewModel(const std::shared_ptr<TraceSourceViewModel>& source,
                                   const std::shared_ptr<UserdataViewModel>&    userdata,
                                   std::function<void()>                        request_capture,
                                   QObject*                                     parent)
    : QObject(parent)
    , source_(source)
    , userdata_(userdata)
    , request_capture_(std::move(request_capture))
{
    files_ = std::make_unique<CaptureFileListModel>();

    connect(source_.get(), &TraceSourceViewModel::ClientStatusChanged, this, [this](const StatusInfo& status) {
        status_text_  = status.text;
        status_color_ = status.color;
        emit statusChanged();
    });

    connect(source_.get(), &TraceSourceViewModel::UiStatusChanged, this, [this](const bool should_enable) {
        capture_enabled_ = should_enable;
        emit statusChanged();
    });

    connect(source_.get(), &TraceSourceViewModel::DisabledReasonChanged, this, [this](const QString& reason) {
        disabled_reason_ = reason;
        emit statusChanged();
    });

    connect(source_.get(), &TraceSourceViewModel::ConnectedProcessTextChanged, this, [this](const QString& text) {
        connected_process_text_ = text;
        emit statusChanged();
    });

    // The trace source reports the path with its macros expanded, which is the folder a
    // capture actually lands in and so the one worth showing.
    connect(source_.get(), &TraceSourceViewModel::OutputPathInfoChanged, this, [this](const OutputPathInfo& info) {
        output_path_     = info.expanded_path;
        raw_output_path_ = info.raw_path;
        emit outputPathChanged();

        // A capture lands in the output directory, so the file list follows it.
        RefreshFiles();
    });

    connect(source_.get(), &TraceSourceViewModel::ProgressInfoStep, this, [this](const ProgressInfo& info) {
        progress_ = info;
        emit progressChanged();
    });

    // The userdata model reports the raw path. It is only used until the trace source has
    // expanded it, so that the field is not empty before an application is chosen.
    connect(userdata_.get(), &UserdataViewModel::OnOutputPathChanged, this, [this](const QString& path) {
        raw_output_path_ = path;
        if (output_path_.isEmpty())
        {
            output_path_ = path;
        }

        emit outputPathChanged();
    });

    connect(userdata_.get(), &UserdataViewModel::OnOutputPathValidated, this, [this](const bool show_warning) {
        output_path_invalid_ = show_warning;
        emit outputPathChanged();
    });

    // A finished, failed or abandoned capture all change what is on disk.
    connect(source_.get(), &TraceSourceViewModel::TraceComplete, this, &CaptureViewModel::RefreshFiles);
    connect(source_.get(), &TraceSourceViewModel::TraceFailed, this, &CaptureViewModel::RefreshFiles);
    connect(source_.get(), &TraceSourceViewModel::TraceAborted, this, &CaptureViewModel::RefreshFiles);
}

CaptureViewModel::~CaptureViewModel() = default;

const std::shared_ptr<TraceSourceViewModel>& CaptureViewModel::Source() const
{
    return source_;
}

const std::shared_ptr<UserdataViewModel>& CaptureViewModel::Userdata() const
{
    return userdata_;
}

QString CaptureViewModel::RawOutputPath() const
{
    return raw_output_path_;
}

bool CaptureViewModel::IsOutputPathInvalid() const
{
    return output_path_invalid_;
}

QString CaptureViewModel::StatusText() const
{
    return status_text_;
}

QColor CaptureViewModel::StatusColor() const
{
    return status_color_;
}

QString CaptureViewModel::ConnectedProcessText() const
{
    return connected_process_text_;
}

bool CaptureViewModel::IsCaptureEnabled() const
{
    return capture_enabled_;
}

QString CaptureViewModel::DisabledReason() const
{
    return disabled_reason_;
}

QString CaptureViewModel::OutputPath() const
{
    return output_path_;
}

void CaptureViewModel::SetOutputPath(const QString& path)
{
    if (path == output_path_)
    {
        return;
    }

    // The userdata view model expands the macros in the path and reports the result back
    // through OnOutputPathChanged, so output_path_ is not written here.
    userdata_->OutputPathChanged(path);
}

bool CaptureViewModel::IsProgressVisible() const
{
    return progress_.show_progress_bar;
}

bool CaptureViewModel::IsProgressCancellable() const
{
    return progress_.can_cancel;
}

qreal CaptureViewModel::Progress() const
{
    return progress_.progress;
}

bool CaptureViewModel::IsProgressIndeterminate() const
{
    return progress_.show_progress_bar && progress_.progress == 0.0F;
}

QString CaptureViewModel::ProgressText() const
{
    return progress_.progress_text;
}

QAbstractItemModel* CaptureViewModel::Files() const
{
    return files_.get();
}

void CaptureViewModel::OpenFile(const int row)
{
    const QString path = files_->PathAt(row);
    if (!path.isEmpty())
    {
        source_->OnOpenFile(path);
    }
}

void CaptureViewModel::DeleteFile(const int row)
{
    const QString path = files_->PathAt(row);
    if (path.isEmpty())
    {
        return;
    }

    source_->OnRemovedFile(path);
    files_->RemoveRow(row);
}

bool CaptureViewModel::IsCapturable() const
{
    return static_cast<bool>(request_capture_);
}

void CaptureViewModel::Capture()
{
    if (request_capture_)
    {
        request_capture_();
    }
}

void CaptureViewModel::Abort()
{
    source_->RequestAbort();
}

void CaptureViewModel::RefreshFiles()
{
    files_->Refresh(output_path_, source_->GetFileExtension());
}
