// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Definition for the model holding the user's connection history.

#ifndef RDP_SOURCE_FRONTEND_MODELS_CONNECTION_HISTORY_MODEL_H_
#define RDP_SOURCE_FRONTEND_MODELS_CONNECTION_HISTORY_MODEL_H_

#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include "tool_wrapper_types.h"

class QSettings;

namespace rdp
{
    class ConnectionModel;
    class SettingsManager;

    /// @brief The list of targets the user has connected to, most recent selection remembered.
    ///
    /// The first entry is always the local connection; the rest are remote targets the user
    /// has connected to at least once. Persisted under the "ConnectionWidget" settings group,
    /// which is the group the Qt Widgets front-end wrote, so existing settings.ini files load
    /// unchanged.
    class ConnectionHistoryModel final : public QAbstractListModel
    {
        Q_OBJECT
        QML_ANONYMOUS

        Q_PROPERTY(int currentIndex READ CurrentIndex WRITE SetCurrentIndex NOTIFY currentIndexChanged)
        Q_PROPERTY(bool autoConnectEnabled READ IsAutoConnectEnabled WRITE SetAutoConnectEnabled NOTIFY autoConnectEnabledChanged)

    public:
        /// @brief The roles exposed to QML.
        enum Roles : int
        {
            kDescriptionRole = Qt::UserRole + 1,  ///< The human-readable description of the target.
            kIsLocalRole,                         ///< true when the entry is the local connection.
            kHostnameRole,                        ///< The remote host name, empty for local.
            kNicknameRole,                        ///< The user's nickname for the target, may be empty.
            kPortRole                             ///< The remote port, 0 for local.
        };
        Q_ENUM(Roles)

        /// @brief Constructor.
        /// @param [in] settings_manager The application settings.
        /// @param [in] connection_model The connection model to drive.
        /// @param [in] parent The parent object.
        ConnectionHistoryModel(std::shared_ptr<SettingsManager> settings_manager,
                               std::weak_ptr<ConnectionModel>   connection_model,
                               QObject*                         parent = nullptr);

        /// @brief Destructor.
        ~ConnectionHistoryModel() override;

        /// @brief Reads the history back out of the application settings.
        void Load();

        /// @brief QAbstractListModel::rowCount() override.
        /// @param [in] parent The parent index.
        /// @return The number of entries in the history.
        int rowCount(const QModelIndex& parent) const override;

        /// @brief QAbstractListModel::data() override.
        /// @param [in] index The index to query.
        /// @param [in] role The role to query.
        /// @return The data for the index and role.
        QVariant data(const QModelIndex& index, int role) const override;

        /// @brief QAbstractListModel::roleNames() override.
        /// @return The role names QML binds to.
        QHash<int, QByteArray> roleNames() const override;

        /// @brief Gets the index of the entry the user last connected with.
        /// @return The current index, or -1 when the history is empty.
        [[nodiscard]] int CurrentIndex() const;

        /// @brief Selects an entry without connecting to it.
        /// @param [in] index The index to select.
        void SetCurrentIndex(int index);

        /// @brief Returns whether RDP reconnects to the last target on start-up.
        /// @return true when auto-connect is enabled.
        [[nodiscard]] bool IsAutoConnectEnabled() const;

        /// @brief Sets whether RDP reconnects to the last target on start-up.
        /// @param [in] enabled true to enable auto-connect.
        void SetAutoConnectEnabled(bool enabled);

        /// @brief Connects to the local target, selecting it in the history.
        Q_INVOKABLE void ConnectLocal();

        /// @brief Connects to a remote target, adding or updating its history entry.
        /// @param [in] hostname The host name or IP.
        /// @param [in] nickname The user's name for the target, may be empty.
        /// @param [in] port The port.
        Q_INVOKABLE void ConnectRemote(const QString& hostname, const QString& nickname, int port);

        /// @brief Connects to the target at the given history index.
        /// @param [in] index The history index to connect to.
        Q_INVOKABLE void ConnectToIndex(int index);

        /// @brief Removes the entry at the given index. The local entry cannot be removed.
        /// @param [in] index The history index to remove.
        Q_INVOKABLE void RemoveAt(int index);

        /// @brief Gets one history entry as a map, so the connection form can populate itself.
        /// @param [in] index The history index.
        /// @return The entry's fields, or an empty map when the index is out of range.
        Q_INVOKABLE QVariantMap EntryAt(int index) const;

        /// @brief Connects to the last used target, if auto-connect is enabled.
        void AttemptAutoConnection();

        /// @brief Connects to a "host" or "host:port" target given on the command line.
        /// @param [in] target The target string.
        void AttemptNewRemoteConnection(const QString& target);

    signals:
        /// @brief Emitted when the selected entry changes.
        void currentIndexChanged();

        /// @brief Emitted when the auto-connect setting changes.
        void autoConnectEnabledChanged();

        /// @brief Emitted when a connection was requested with an unusable host name.
        void invalidHostName();

    private slots:
        /// @brief Writes the history into the application settings.
        /// @param [in] settings The settings to write to.
        void OnSaveSettings(QSettings* settings) const;

    private:
        /// @brief One remembered target. A local entry has an empty hostname.
        struct Entry
        {
            bool    is_local = false;  ///< true for the local connection entry.
            QString hostname;          ///< The remote host name.
            QString nickname;          ///< The user's name for the target.
            int     port = 0;          ///< The remote port.

            /// @brief Gets the description shown in the history list.
            /// @return The description.
            [[nodiscard]] QString GetDescription() const;
        };

        /// @brief Finds a remote entry by host and port, ignoring the nickname.
        /// @param [in] hostname The host name to match.
        /// @param [in] port The port to match.
        /// @return The index of the match, or -1.
        [[nodiscard]] int IndexOfRemote(const QString& hostname, int port) const;

        static constexpr const char* kSettingsPrefix = "ConnectionWidget";  ///< Settings group, kept for compatibility.
        static constexpr int         kDefaultPort    = 27300;               ///< The port assumed when a target omits one.

        std::vector<Entry>               entries_;                    ///< The remembered targets, local first.
        std::shared_ptr<SettingsManager> settings_manager_;           ///< The application settings.
        std::weak_ptr<ConnectionModel>   connection_model_;           ///< The connection model this drives.
        int                              current_index_        = 0;   ///< The entry the user last connected with.
        bool                             auto_connect_enabled_ = true;  ///< true when RDP reconnects on start-up.
    };
}  // namespace rdp

#endif
