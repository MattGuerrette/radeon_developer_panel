// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Definition for the proxy that splits the modules by category.

#ifndef RDP_SOURCE_FRONTEND_MODELS_MODULE_CATEGORY_PROXY_MODEL_H_
#define RDP_SOURCE_FRONTEND_MODELS_MODULE_CATEGORY_PROXY_MODEL_H_

#include <QSortFilterProxyModel>

namespace rdp
{
    /// @brief Shows the modules of one category, optionally only the enabled ones.
    ///
    /// The CAPTURE view is tied to application connections and the SYSTEM view is not, so
    /// each drives its own instance of this over the single ModuleModel. The CAPTURE view
    /// needs two: every capture module for the "available features" pane, and just the
    /// enabled ones for the tab bar.
    ///
    /// All instances read the same ModuleModel, so a row in one does not line up with the
    /// same row in another; SourceRow() maps back for callers that have to act on a module.
    class ModuleCategoryProxyModel final : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        /// @param [in] system_modules true to show the system modules, false for the capture modules.
        /// @param [in] enabled_only true to show only the modules that are currently enabled.
        /// @param [in] parent The parent object.
        explicit ModuleCategoryProxyModel(bool system_modules, bool enabled_only = false, QObject* parent = nullptr);

        /// @brief Maps a row of this proxy back onto the source module model.
        /// @param [in] row The proxy row.
        /// @return The row in the source model, or -1 when out of range.
        int SourceRow(int row) const;

    protected:
        /// @brief QSortFilterProxyModel::filterAcceptsRow() override.
        /// @param [in] source_row The row in the source model.
        /// @param [in] source_parent The parent of the row in the source model.
        /// @return true when the row's module belongs in this proxy.
        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    private:
        bool system_modules_ = false;  ///< true when this proxy shows the system modules.
        bool enabled_only_   = false;  ///< true when this proxy hides the disabled modules.
    };
}  // namespace rdp

#endif
