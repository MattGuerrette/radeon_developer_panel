// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the model the device clocks QML view binds to.

#include "device_clocks_view_model.h"

#include <utility>

#include <QVariantMap>

#include "clock_mode_model.h"
#include "device_clocks_gpu_model.h"
#include "notifications.h"

DeviceClocksGpuListModel::DeviceClocksGpuListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

DeviceClocksGpuListModel::~DeviceClocksGpuListModel() = default;

void DeviceClocksGpuListModel::SetGpus(const QVector<std::shared_ptr<DeviceClocksGpuModel>>& gpus)
{
    beginResetModel();
    gpus_ = gpus;
    endResetModel();
}

bool DeviceClocksGpuListModel::RequestMode(const int row, const int mode)
{
    if (row < 0 || row >= gpus_.size())
    {
        return false;
    }

    const bool accepted = gpus_[row]->RequestMode(mode);

    // The active mode is read back from the driver rather than assumed, so a refused
    // request leaves the view showing what the GPU is actually doing.
    RefreshCurrentMode(row);

    return accepted;
}

QString DeviceClocksGpuListModel::GpuName(const int row) const
{
    if (row < 0 || row >= gpus_.size())
    {
        return {};
    }

    return gpus_[row]->GetGpuName();
}

void DeviceClocksGpuListModel::RefreshCurrentMode(const int row)
{
    const QModelIndex changed = index(row, 0, {});
    emit              dataChanged(changed, changed, {kCurrentModeRole});
}

int DeviceClocksGpuListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(gpus_.size());
}

QVariant DeviceClocksGpuListModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= gpus_.size())
    {
        return {};
    }

    const std::shared_ptr<DeviceClocksGpuModel>& gpu = gpus_[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
    case kNameRole:
        return gpu->GetGpuName();

    case kModesRole:
    {
        QVariantList modes;
        for (const std::shared_ptr<ClockModeModel>& mode : gpu->GetModeModels())
        {
            modes.append(QVariantMap{{"name", mode->GetName()},
                                     {"description", mode->GetDescription()},
                                     {"shaderClock", mode->GetGpuFreqStr()},
                                     {"memoryClock", mode->GetMemFreqStr()}});
        }

        return modes;
    }

    case kCurrentModeRole:
        return gpu->GetCurrentMode();

    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceClocksGpuListModel::roleNames() const
{
    return {{kNameRole, "name"}, {kModesRole, "modes"}, {kCurrentModeRole, "currentMode"}};
}

DeviceClocksViewModel::DeviceClocksViewModel(std::shared_ptr<DeviceClocksModel> model, QObject* parent)
    : QObject(parent)
    , model_(std::move(model))
{
    gpus_ = std::make_unique<DeviceClocksGpuListModel>();

    // The manager reports from its own thread, so the connection is queued onto the UI
    // thread the list model lives on.
    connect(model_.get(), &DeviceClocksModel::GpusChanged, this, &DeviceClocksViewModel::OnGpusChanged, Qt::QueuedConnection);
}

DeviceClocksViewModel::~DeviceClocksViewModel() = default;

QAbstractItemModel* DeviceClocksViewModel::Gpus() const
{
    return gpus_.get();
}

QString DeviceClocksViewModel::EmptyMessage() const
{
    if (!connected_)
    {
        return QObject::tr("Connect to a system to begin using device clocks.");
    }

    if (gpus_->rowCount({}) == 0)
    {
        return QObject::tr("The system does not have any GPUs that can be used with device clocks.");
    }

    return {};
}

void DeviceClocksViewModel::RequestMode(const int row, const int mode)
{
    if (!gpus_->RequestMode(row, mode))
    {
        rdp::Notifications::Critical(QObject::tr("Error"), QObject::tr("Failed to update the clock mode for %1").arg(gpus_->GpuName(row)));
    }
}

void DeviceClocksViewModel::OnGpusChanged(QVector<std::shared_ptr<DeviceClocksGpuModel>> gpus, const bool is_connected)
{
    connected_ = is_connected;
    gpus_->SetGpus(is_connected ? gpus : QVector<std::shared_ptr<DeviceClocksGpuModel>>{});

    emit stateChanged();
}
