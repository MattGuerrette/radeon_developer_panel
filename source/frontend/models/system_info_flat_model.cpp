// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the flattened view of the system information tree.

#include "models/system_info_flat_model.h"

#include <utility>

#include "models/system_info_model.h"

namespace rdp
{
    SystemInfoFlatModel::SystemInfoFlatModel(std::shared_ptr<SystemInfoModel> source, QObject* parent)
        : QAbstractListModel(parent)
        , source_(std::move(source))
    {
        connect(source_.get(), &SystemInfoModel::SystemInfoChanged, this, &SystemInfoFlatModel::Rebuild);
        connect(source_.get(), &SystemInfoModel::modelReset, this, &SystemInfoFlatModel::Rebuild);

        Rebuild();
    }

    SystemInfoFlatModel::~SystemInfoFlatModel() = default;

    void SystemInfoFlatModel::Rebuild()
    {
        beginResetModel();
        rows_.clear();
        Append({}, 0);
        endResetModel();
    }

    void SystemInfoFlatModel::Append(const QModelIndex& parent, const int depth)
    {
        const int count = source_->rowCount(parent);
        for (int i = 0; i < count; i++)
        {
            const QModelIndex key_index   = source_->index(i, SystemInfoModel::kSystemInfoItemColumnKey, parent);
            const QModelIndex value_index = source_->index(i, SystemInfoModel::kSystemInfoItemColumnValue, parent);

            const bool has_children = source_->rowCount(key_index) > 0;

            rows_.push_back({key_index.data(Qt::DisplayRole).toString(),
                             has_children ? QString() : value_index.data(Qt::DisplayRole).toString(),
                             depth,
                             has_children});

            if (has_children)
            {
                Append(key_index, depth + 1);
            }
        }
    }

    int SystemInfoFlatModel::rowCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(rows_.size());
    }

    QVariant SystemInfoFlatModel::data(const QModelIndex& index, const int role) const
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(rows_.size()))
        {
            return {};
        }

        const Row& row = rows_[static_cast<size_t>(index.row())];

        switch (role)
        {
        case Qt::DisplayRole:
        case kKeyRole:
            return row.key;
        case kValueRole:
            return row.value;
        case kDepthRole:
            return row.depth;
        case kIsSectionRole:
            return row.is_section;
        default:
            return {};
        }
    }

    QHash<int, QByteArray> SystemInfoFlatModel::roleNames() const
    {
        return {{kKeyRole, "key"}, {kValueRole, "value"}, {kDepthRole, "depth"}, {kIsSectionRole, "isSection"}};
    }
}  // namespace rdp
