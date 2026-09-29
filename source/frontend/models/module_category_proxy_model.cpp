// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the proxy that splits the modules by category.

#include "models/module_category_proxy_model.h"

#include "models/module_model.h"

namespace rdp
{
    ModuleCategoryProxyModel::ModuleCategoryProxyModel(const bool system_modules, const bool enabled_only, QObject* parent)
        : QSortFilterProxyModel(parent)
        , system_modules_(system_modules)
        , enabled_only_(enabled_only)
    {
        // ModuleModel signals an enable/disable as a data change, which does not re-run the
        // filter on its own.
        setDynamicSortFilter(true);
    }

    int ModuleCategoryProxyModel::SourceRow(const int row) const
    {
        const QModelIndex proxy_index = index(row, 0, {});
        if (!proxy_index.isValid())
        {
            return -1;
        }

        return mapToSource(proxy_index).row();
    }

    bool ModuleCategoryProxyModel::filterAcceptsRow(const int source_row, const QModelIndex& source_parent) const
    {
        const QAbstractItemModel* source = sourceModel();
        if (source == nullptr)
        {
            return false;
        }

        const QModelIndex source_index = source->index(source_row, 0, source_parent);

        if (source_index.data(ModuleModel::kIsSystemModuleRole).toBool() != system_modules_)
        {
            return false;
        }

        return !enabled_only_ || source_index.data(ModuleModel::kEnabledRole).toBool();
    }
}  // namespace rdp
