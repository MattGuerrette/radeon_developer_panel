// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the model behind the output log view.

#include "logging/logging_view_model.h"

#include <utility>

#include <qt_common/utils/qt_util.h>

#include "logging/logging_model.h"

namespace rdp
{
    LoggingFilterProxyModel::LoggingFilterProxyModel(QObject* parent)
        : QSortFilterProxyModel(parent)
    {
    }

    bool LoggingFilterProxyModel::filterAcceptsRow(const int source_row, const QModelIndex& source_parent) const
    {
        const QAbstractItemModel* source = sourceModel();
        if (source == nullptr)
        {
            return false;
        }

        const QModelIndex source_index = source->index(source_row, 0, {});
        if (!source_index.isValid())
        {
            return false;
        }

        if (!source_.isEmpty() && source->data(source_index.siblingAtColumn(LoggingModel::Columns::kSource)).toString() != source_)
        {
            return false;
        }

        if (pid_ != kLoggingInvalidPid && source->data(source_index.siblingAtColumn(LoggingModel::Columns::kPid)).toUInt() != pid_)
        {
            return false;
        }

        if (umd_id_ != kLoggingInvalidUmdId && source->data(source_index.siblingAtColumn(LoggingModel::Columns::kUmdId)).toUInt() != umd_id_)
        {
            return false;
        }

        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    void LoggingFilterProxyModel::SetFilters(const QString& source, const uint32_t pid, const DDConnectionId umd_id)
    {
        source_ = source;
        pid_    = pid;
        umd_id_ = umd_id;

        invalidateFilter();
    }

    QVariant LoggingFilterProxyModel::data(const QModelIndex& index, const int role) const
    {
        // LoggingModel is column-based; QML reads a row through roles, so each role maps
        // onto the sibling cell it corresponds to.
        const auto column_for_role = [](const int queried_role) -> int {
            switch (queried_role)
            {
            case kTimestampRole:
                return LoggingModel::Columns::kTimestamp;
            case kLevelRole:
            case kLevelNameRole:
                return LoggingModel::Columns::kLevel;
            case kSourceRole:
                return LoggingModel::Columns::kSource;
            case kPidRole:
                return LoggingModel::Columns::kPid;
            case kUmdIdRole:
                return LoggingModel::Columns::kUmdId;
            case kMessageRole:
                return LoggingModel::Columns::kMessage;
            default:
                return -1;
            }
        };

        const int column = column_for_role(role);
        if (column < 0)
        {
            return QSortFilterProxyModel::data(index, role);
        }

        const QModelIndex cell = index.siblingAtColumn(column);

        // The severity is carried on Qt::UserRole so the view can colour the row; every
        // other role is the cell's display text.
        if (role == kLevelRole)
        {
            return QSortFilterProxyModel::data(cell, Qt::UserRole);
        }

        return QSortFilterProxyModel::data(cell, Qt::DisplayRole);
    }

    QHash<int, QByteArray> LoggingFilterProxyModel::roleNames() const
    {
        return {{kTimestampRole, "timestamp"},
                {kLevelRole, "level"},
                {kLevelNameRole, "levelName"},
                {kSourceRole, "source"},
                {kPidRole, "pid"},
                {kUmdIdRole, "umdId"},
                {kMessageRole, "message"}};
    }

    LoggingViewModel::LoggingViewModel(std::shared_ptr<LoggingModel> logging_model, QObject* parent)
        : QObject(parent)
        , logging_model_(std::move(logging_model))
    {
        entries_ = std::make_unique<LoggingFilterProxyModel>();
        entries_->setSourceModel(logging_model_.get());

        connect(logging_model_.get(), &LoggingModel::LogFilePathChanged, this, &LoggingViewModel::logFilePathChanged);
    }

    LoggingViewModel::~LoggingViewModel() = default;

    QAbstractItemModel* LoggingViewModel::Entries() const
    {
        return entries_.get();
    }

    QAbstractItemModel* LoggingViewModel::SourceFilters() const
    {
        return logging_model_->GetSourceFilterModel().get();
    }

    QAbstractItemModel* LoggingViewModel::PidFilters() const
    {
        return logging_model_->GetPidFilterModel().get();
    }

    QAbstractItemModel* LoggingViewModel::UmdIdFilters() const
    {
        return logging_model_->GetUmdIdFilterModel().get();
    }

    int LoggingViewModel::SourceFilterIndex() const
    {
        return source_filter_index_;
    }

    void LoggingViewModel::SetSourceFilterIndex(const int index)
    {
        if (index == source_filter_index_)
        {
            return;
        }

        source_filter_index_ = index;
        ApplyFilters();
    }

    int LoggingViewModel::PidFilterIndex() const
    {
        return pid_filter_index_;
    }

    void LoggingViewModel::SetPidFilterIndex(const int index)
    {
        if (index == pid_filter_index_)
        {
            return;
        }

        pid_filter_index_ = index;
        ApplyFilters();
    }

    int LoggingViewModel::UmdIdFilterIndex() const
    {
        return umd_id_filter_index_;
    }

    void LoggingViewModel::SetUmdIdFilterIndex(const int index)
    {
        if (index == umd_id_filter_index_)
        {
            return;
        }

        umd_id_filter_index_ = index;
        ApplyFilters();
    }

    bool LoggingViewModel::IsUmdFilterEnabled() const
    {
        const auto&       pid_model = logging_model_->GetPidFilterModel();
        const QModelIndex pid_index = pid_model->index(pid_filter_index_, 0, {});

        return pid_model->GetFilterAtIndex(pid_index) != kLoggingInvalidPid;
    }

    QString LoggingViewModel::LogFilePath() const
    {
        return logging_model_->GetLogFilePath();
    }

    bool LoggingViewModel::HasLogFile() const
    {
        return !logging_model_->GetLogFilePath().isEmpty();
    }

    void LoggingViewModel::OpenLogFileLocation() const
    {
        QtCommon::QtUtils::BrowseToFile(logging_model_->GetLogFilePath());
    }

    void LoggingViewModel::ApplyFilters()
    {
        const auto& source_model = logging_model_->GetSourceFilterModel();
        const auto& pid_model    = logging_model_->GetPidFilterModel();
        const auto& umd_model    = logging_model_->GetUmdIdFilterModel();

        const QString  source       = source_model->GetFilterAtIndex(source_model->index(source_filter_index_, 0, {}));
        const uint32_t selected_pid = pid_model->GetFilterAtIndex(pid_model->index(pid_filter_index_, 0, {}));

        DDConnectionId selected_umd_id = kLoggingInvalidUmdId;

        if (selected_pid == kLoggingInvalidPid)
        {
            // With every process shown, a single UMD connection is not a meaningful filter.
            umd_id_filter_index_ = 0;
        }
        else
        {
            selected_umd_id = umd_model->GetFilterAtIndex(umd_model->index(umd_id_filter_index_, 0, {}));
        }

        // The list of UMD connections is per process, so it is rebuilt whenever the
        // selected process moves, and the selection resets with it.
        if (selected_pid != last_selected_pid_)
        {
            logging_model_->RebuildUmdFilters(selected_pid, kLoggingInvalidUmdId);
            last_selected_pid_   = selected_pid;
            umd_id_filter_index_ = 0;
            selected_umd_id      = kLoggingInvalidUmdId;
        }

        entries_->SetFilters(source, selected_pid, selected_umd_id);

        emit filtersChanged();
    }
}  // namespace rdp
