// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the model holding the user's connection history.

#include "models/connection_history_model.h"

#include <utility>

#include <QSettings>

#include "logging/logging_manager.h"
#include "models/connection_model.h"
#include "settings_manager.h"

static constexpr const char* kEnableAutoConnectSettingKey     = "EnableAutoConnectToLastConnection";
static constexpr const char* kDisableClientTimeoutSettingsKey = "DisableClientTimeout";

namespace rdp
{
    QString ConnectionHistoryModel::Entry::GetDescription() const
    {
        if (is_local)
        {
            return LocalConnectionInfo::GetDescription();
        }

        RemoteConnectionInfo info;
        info.hostname = hostname;
        info.nickname = nickname;
        info.port     = port;

        return info.GetDescription();
    }

    ConnectionHistoryModel::ConnectionHistoryModel(std::shared_ptr<SettingsManager> settings_manager,
                                                   std::weak_ptr<ConnectionModel>   connection_model,
                                                   QObject*                         parent)
        : QAbstractListModel(parent)
        , settings_manager_(std::move(settings_manager))
        , connection_model_(std::move(connection_model))
    {
        // The local connection is always present, whether or not anything was ever saved.
        entries_.push_back({true, {}, {}, 0});

        connect(settings_manager_.get(), &SettingsManager::SettingsSaved, this, &ConnectionHistoryModel::OnSaveSettings);
    }

    ConnectionHistoryModel::~ConnectionHistoryModel() = default;

    void ConnectionHistoryModel::Load()
    {
        RDP_LOG_INFO("Loading settings group: [{}]", kSettingsPrefix);

        QSettings* settings = settings_manager_->GetSettings();
        Q_ASSERT(settings != nullptr);

        if (settings->status() != QSettings::Status::NoError)
        {
            RDP_LOG_ERROR("Failure: Invalid QSettings");
            return;
        }

        settings->beginGroup(kSettingsPrefix);

        if (!settings->value("saved", false).toBool())
        {
            RDP_LOG_WARN("No settings present!");
            settings->endGroup();
            return;
        }

        const int saved_index        = settings->value("lastUsedIndex", 0).toInt();
        const int history_item_count = settings->value("historySize", 0).toInt();

        std::vector<Entry> loaded;
        for (int i = 0; i < history_item_count; i++)
        {
            settings->beginGroup(QString("history/%1/").arg(i));

            const QString type_string = settings->value("type", "").toString();
            if (type_string == "local")
            {
                loaded.push_back({true, {}, {}, 0});
            }
            else if (type_string == "remote")
            {
                const QString hostname = settings->value("hostname", "").toString();
                const QString nickname = settings->value("nickname", "").toString();
                const int     port     = settings->value("port", 0).toInt();

                if (!hostname.isEmpty() && port != 0)
                {
                    loaded.push_back({false, hostname, nickname, port});
                }
            }
            else if (!type_string.isEmpty())
            {
                RDP_LOG_WARN("Unrecognized connection type: {}", type_string.toStdString().c_str());
            }

            settings->endGroup();
        }

        if (!loaded.empty())
        {
            beginResetModel();
            entries_ = std::move(loaded);
            endResetModel();
        }

        if (settings->contains(kEnableAutoConnectSettingKey))
        {
            auto_connect_enabled_ = settings->value(kEnableAutoConnectSettingKey).toBool();
            emit autoConnectEnabledChanged();
        }

        const bool disable_client_timeout = settings->value(kDisableClientTimeoutSettingsKey, false).toBool();
        if (const auto connection_model = connection_model_.lock())
        {
            connection_model->SetDisableClientTimeout(disable_client_timeout);
        }

        settings->endGroup();

        SetCurrentIndex(saved_index);
    }

    int ConnectionHistoryModel::rowCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(entries_.size());
    }

    QVariant ConnectionHistoryModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(entries_.size()))
        {
            return {};
        }

        const Entry& entry = entries_[static_cast<size_t>(index.row())];

        switch (role)
        {
        case Qt::DisplayRole:
        case kDescriptionRole:
            return entry.GetDescription();
        case kIsLocalRole:
            return entry.is_local;
        case kHostnameRole:
            return entry.hostname;
        case kNicknameRole:
            return entry.nickname;
        case kPortRole:
            return entry.port;
        default:
            return {};
        }
    }

    QHash<int, QByteArray> ConnectionHistoryModel::roleNames() const
    {
        return {{kDescriptionRole, "description"},
                {kIsLocalRole, "isLocal"},
                {kHostnameRole, "hostname"},
                {kNicknameRole, "nickname"},
                {kPortRole, "port"}};
    }

    int ConnectionHistoryModel::CurrentIndex() const
    {
        return current_index_;
    }

    void ConnectionHistoryModel::SetCurrentIndex(int index)
    {
        const int clamped = (index >= 0 && index < static_cast<int>(entries_.size())) ? index : 0;
        if (clamped == current_index_)
        {
            return;
        }

        current_index_ = clamped;
        emit currentIndexChanged();
    }

    bool ConnectionHistoryModel::IsAutoConnectEnabled() const
    {
        return auto_connect_enabled_;
    }

    void ConnectionHistoryModel::SetAutoConnectEnabled(bool enabled)
    {
        if (enabled == auto_connect_enabled_)
        {
            return;
        }

        auto_connect_enabled_ = enabled;
        emit autoConnectEnabledChanged();
    }

    int ConnectionHistoryModel::IndexOfRemote(const QString& hostname, int port) const
    {
        for (size_t i = 0; i < entries_.size(); i++)
        {
            const Entry& entry = entries_[i];
            if (!entry.is_local && entry.hostname == hostname && entry.port == port)
            {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    void ConnectionHistoryModel::ConnectLocal()
    {
        for (size_t i = 0; i < entries_.size(); i++)
        {
            if (entries_[i].is_local)
            {
                SetCurrentIndex(static_cast<int>(i));
                break;
            }
        }

        settings_manager_->SaveSettings();

        if (const auto connection_model = connection_model_.lock())
        {
            connection_model->ConnectToolLocal();
        }
    }

    void ConnectionHistoryModel::ConnectRemote(const QString& hostname, const QString& nickname, int port)
    {
        if (hostname.isEmpty())
        {
            emit invalidHostName();
            return;
        }

        // Match on host and port only: reconnecting with a new nickname renames the entry
        // rather than adding a second one for the same machine.
        const int existing = IndexOfRemote(hostname, port);
        if (existing < 0)
        {
            const int row = static_cast<int>(entries_.size());
            beginInsertRows({}, row, row);
            entries_.push_back({false, hostname, nickname, port});
            endInsertRows();
            SetCurrentIndex(row);
        }
        else
        {
            entries_[static_cast<size_t>(existing)].nickname = nickname;
            const QModelIndex changed                        = index(existing, 0, {});
            emit dataChanged(changed, changed, {Qt::DisplayRole, kDescriptionRole, kNicknameRole});
            SetCurrentIndex(existing);
        }

        settings_manager_->SaveSettings();

        RemoteConnectionInfo info;
        info.hostname = hostname;
        info.nickname = nickname;
        info.port     = port;

        if (const auto connection_model = connection_model_.lock())
        {
            connection_model->ConnectToolRemote(info);
        }
    }

    void ConnectionHistoryModel::ConnectToIndex(int index)
    {
        if (index < 0 || index >= static_cast<int>(entries_.size()))
        {
            return;
        }

        const Entry entry = entries_[static_cast<size_t>(index)];
        if (entry.is_local)
        {
            ConnectLocal();
        }
        else
        {
            ConnectRemote(entry.hostname, entry.nickname, entry.port);
        }
    }

    void ConnectionHistoryModel::RemoveAt(int index)
    {
        if (index < 0 || index >= static_cast<int>(entries_.size()) || entries_[static_cast<size_t>(index)].is_local)
        {
            return;
        }

        beginRemoveRows({}, index, index);
        entries_.erase(entries_.begin() + index);
        endRemoveRows();

        if (current_index_ >= static_cast<int>(entries_.size()))
        {
            SetCurrentIndex(static_cast<int>(entries_.size()) - 1);
        }

        settings_manager_->SaveSettings();
    }

    QVariantMap ConnectionHistoryModel::EntryAt(const int index) const
    {
        if (index < 0 || index >= static_cast<int>(entries_.size()))
        {
            return {};
        }

        const Entry& entry = entries_[static_cast<size_t>(index)];

        return {{"isLocal", entry.is_local}, {"hostname", entry.hostname}, {"nickname", entry.nickname}, {"port", entry.port}};
    }

    void ConnectionHistoryModel::AttemptAutoConnection()
    {
        if (!auto_connect_enabled_ || entries_.empty())
        {
            return;
        }

        ConnectToIndex(current_index_);
    }

    void ConnectionHistoryModel::AttemptNewRemoteConnection(const QString& target)
    {
        QString hostname = target;
        int     port     = kDefaultPort;

        const QStringList target_parts = target.split(":");
        if (target_parts.length() == 2)
        {
            hostname = target_parts.value(0);
            port     = target_parts.value(1).toInt();
        }

        ConnectRemote(hostname, QString(), port);
    }

    void ConnectionHistoryModel::OnSaveSettings(QSettings* settings) const
    {
        RDP_LOG_INFO("Saving settings group: [{}]", kSettingsPrefix);

        Q_ASSERT(settings != nullptr);

        if (settings->status() != QSettings::Status::NoError)
        {
            RDP_LOG_ERROR("Failure: Invalid QSettings");
            return;
        }

        settings->beginGroup(kSettingsPrefix);

        settings->setValue("saved", true);
        settings->setValue("historySize", static_cast<int>(entries_.size()));
        settings->setValue("lastUsedIndex", current_index_ >= 0 ? current_index_ : 0);

        for (size_t i = 0; i < entries_.size(); i++)
        {
            settings->beginGroup(QString("history/%1/").arg(i));

            // Clear any lingering values at this index before writing this entry's shape.
            settings->remove("type");
            settings->remove("hostname");
            settings->remove("nickname");
            settings->remove("port");
            settings->remove("privateBusId");

            const Entry& entry = entries_[i];
            if (entry.is_local)
            {
                settings->setValue("type", "local");
            }
            else
            {
                settings->setValue("type", "remote");
                settings->setValue("hostname", entry.hostname);
                settings->setValue("nickname", entry.nickname);
                settings->setValue("port", entry.port);
            }

            settings->endGroup();
        }

        settings->setValue(kEnableAutoConnectSettingKey, auto_connect_enabled_);

        if (const auto connection_model = connection_model_.lock())
        {
            settings->setValue(kDisableClientTimeoutSettingsKey, connection_model->GetIsClientTimeoutDisabled());
        }

        settings->endGroup();
    }
}  // namespace rdp
