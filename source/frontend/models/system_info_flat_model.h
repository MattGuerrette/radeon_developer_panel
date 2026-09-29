// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Definition for the flattened view of the system information tree.

#ifndef RDP_SOURCE_FRONTEND_MODELS_SYSTEM_INFO_FLAT_MODEL_H_
#define RDP_SOURCE_FRONTEND_MODELS_SYSTEM_INFO_FLAT_MODEL_H_

#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QString>

namespace rdp
{
    class SystemInfoModel;

    /// @brief Presents the system information tree as a flat list of rows.
    ///
    /// The system information is a tree of arbitrary depth ("Devices" -> a device -> its
    /// fields), which a QML Repeater cannot walk. Flattening it keeps the depth as a role,
    /// so the view can indent and can style the rows that only introduce a group.
    class SystemInfoFlatModel final : public QAbstractListModel
    {
        Q_OBJECT

    public:
        /// @brief The roles the system information view binds to.
        enum Roles : int
        {
            kKeyRole = Qt::UserRole + 1,  ///< The field name, or the group's title.
            kValueRole,                   ///< The field's value, empty for a group.
            kDepthRole,                   ///< How deeply the row is nested, 0 for a top-level group.
            kIsSectionRole                ///< true when the row introduces a group rather than carrying a value.
        };
        Q_ENUM(Roles)

        /// @brief Constructor.
        /// @param [in] source The system information tree.
        /// @param [in] parent The parent object.
        explicit SystemInfoFlatModel(std::shared_ptr<SystemInfoModel> source, QObject* parent = nullptr);

        /// @brief Destructor.
        ~SystemInfoFlatModel() override;

        /// @brief QAbstractListModel::rowCount() override.
        /// @param [in] parent The parent index.
        /// @return The number of flattened rows.
        int rowCount(const QModelIndex& parent) const override;

        /// @brief QAbstractListModel::data() override.
        /// @param [in] index The index to query.
        /// @param [in] role The role to query.
        /// @return The data for the index and role.
        QVariant data(const QModelIndex& index, int role) const override;

        /// @brief QAbstractListModel::roleNames() override.
        /// @return The role names the system information view binds to.
        QHash<int, QByteArray> roleNames() const override;

    private slots:
        /// @brief Re-walks the tree after the system information has changed.
        void Rebuild();

    private:
        /// @brief One flattened row.
        struct Row
        {
            QString key;                 ///< The field name, or the group's title.
            QString value;               ///< The field's value, empty for a group.
            int     depth      = 0;      ///< How deeply the row is nested.
            bool    is_section = false;  ///< true when the row only introduces a group.
        };

        /// @brief Appends a subtree to the flattened rows.
        /// @param [in] parent The index whose children to append.
        /// @param [in] depth The depth to record for those children.
        void Append(const QModelIndex& parent, int depth);

        std::vector<Row>                 rows_;    ///< The flattened rows, in tree order.
        std::shared_ptr<SystemInfoModel> source_;  ///< The system information tree.
    };
}  // namespace rdp

#endif
