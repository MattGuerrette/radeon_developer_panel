// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Definition for the model the device clocks QML view binds to.

#ifndef RDP_SOURCE_MODULES_DEVICECLOCKS_SRC_GUI_DEVICE_CLOCKS_VIEW_MODEL_H_
#define RDP_SOURCE_MODULES_DEVICECLOCKS_SRC_GUI_DEVICE_CLOCKS_VIEW_MODEL_H_

#include <memory>

#include <QAbstractListModel>
#include <QString>
#include <QVariantList>
#include <QVector>
#include <QtQml/qqmlregistration.h>

#include "device_clocks_model.h"

/// @brief The GPUs the device clocks module can drive, one per row.
class DeviceClocksGpuListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    /// @brief The roles the device clocks view binds to.
    enum Roles : int
    {
        kNameRole = Qt::UserRole + 1,  ///< The GPU's name.
        kModesRole,                    ///< The GPU's clock modes, as a list of maps.
        kCurrentModeRole               ///< The row of the active mode, or -1 when it is not known.
    };
    Q_ENUM(Roles)

    /// @brief Constructor.
    /// @param [in] parent The parent object.
    explicit DeviceClocksGpuListModel(QObject* parent = nullptr);

    /// @brief Destructor.
    ~DeviceClocksGpuListModel() override;

    /// @brief Replaces the GPUs shown.
    /// @param [in] gpus The GPUs the device clocks manager reported.
    void SetGpus(const QVector<std::shared_ptr<DeviceClocksGpuModel>>& gpus);

    /// @brief Asks the driver to put a GPU into one of its clock modes.
    /// @param [in] row The GPU's row.
    /// @param [in] mode The row of the mode to request.
    /// @return true when the driver accepted the mode.
    bool RequestMode(int row, int mode);

    /// @brief Gets the name of the GPU in the given row.
    /// @param [in] row The GPU's row.
    /// @return The GPU's name, or empty when the row is out of range.
    [[nodiscard]] QString GpuName(int row) const;

    /// @brief QAbstractListModel::rowCount() override.
    /// @param [in] parent The parent index.
    /// @return The number of GPUs.
    int rowCount(const QModelIndex& parent) const override;

    /// @brief QAbstractListModel::data() override.
    /// @param [in] index The index to query.
    /// @param [in] role The role to query.
    /// @return The data for the index and role.
    QVariant data(const QModelIndex& index, int role) const override;

    /// @brief QAbstractListModel::roleNames() override.
    /// @return The role names the device clocks view binds to.
    QHash<int, QByteArray> roleNames() const override;

private:
    /// @brief Re-reads the active mode of one GPU and tells the view.
    /// @param [in] row The GPU's row.
    void RefreshCurrentMode(int row);

    QVector<std::shared_ptr<DeviceClocksGpuModel>> gpus_;  ///< The GPUs, in the order reported.
};

/// @brief The model the device clocks QML view binds to.
class DeviceClocksViewModel final : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(QAbstractItemModel* gpus READ Gpus CONSTANT)
    /// Empty while there is something to show; otherwise says why there is not.
    Q_PROPERTY(QString emptyMessage READ EmptyMessage NOTIFY stateChanged)

public:
    /// @brief Constructor.
    /// @param [in] model The device clocks model.
    /// @param [in] parent The parent object.
    explicit DeviceClocksViewModel(std::shared_ptr<DeviceClocksModel> model, QObject* parent = nullptr);

    /// @brief Destructor.
    ~DeviceClocksViewModel() override;

    /// @brief Gets the GPUs the module can drive.
    /// @return The GPU list model.
    [[nodiscard]] QAbstractItemModel* Gpus() const;

    /// @brief Gets the message shown when there are no GPUs to show.
    /// @return The message, empty when there are GPUs.
    [[nodiscard]] QString EmptyMessage() const;

    /// @brief Asks the driver to put a GPU into one of its clock modes.
    ///
    /// Raises a notification when the driver refuses, which is how the Qt Widgets view
    /// reported the same failure.
    /// @param [in] row The GPU's row.
    /// @param [in] mode The row of the mode to request.
    Q_INVOKABLE void RequestMode(int row, int mode);

signals:
    /// @brief Emitted when the set of GPUs, or the connection behind them, changes.
    void stateChanged();

private slots:
    /// @brief Called when the device clocks manager reports a different set of GPUs.
    /// @param [in] gpus The GPUs that are available.
    /// @param [in] is_connected true when connected to a Radeon Developer Service.
    void OnGpusChanged(QVector<std::shared_ptr<DeviceClocksGpuModel>> gpus, bool is_connected);

private:
    std::shared_ptr<DeviceClocksModel>          model_;         ///< The device clocks model.
    std::unique_ptr<DeviceClocksGpuListModel>   gpus_;          ///< The GPUs the module can drive.
    bool                                        connected_ = false;  ///< true while connected to a service.
};

#endif
