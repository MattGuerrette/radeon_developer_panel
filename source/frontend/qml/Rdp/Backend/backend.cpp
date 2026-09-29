// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of the root object the QML front-end binds to.

#include "backend.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QJSEngine>
#include <QQmlEngine>
#include <QUrl>

#include "bug/bug_report_generator.h"
#include "logging/logging_manager.h"
#include "notifications.h"

#include <utility>

#include "models/api_model.h"
#include "models/app_settings_model.h"
#include "models/application_model.h"
#include "models/api_proxy_model.h"
#include "models/blocklist_model.h"
#include "models/connection_history_model.h"
#include "models/connection_model.h"
#include "models/module_category_proxy_model.h"
#include "models/module_model.h"
#include "models/preset_model.h"
#include "models/sidebar/auto_connect_model.h"
#include "models/system_info_flat_model.h"
#include "models/system_info_model.h"
#include "models/timeout_model.h"
#include "settings_manager.h"
#include "version.h"

namespace rdp
{
    namespace
    {
        constexpr const char* kHelpFilePath = "/help/rdp/index.html";  ///< The bundled documentation, relative to the executable.
    }

    Backend::Backend() = default;

    Backend::~Backend() = default;

    Backend& Backend::Instance()
    {
        static Backend instance;
        return instance;
    }

    Backend* Backend::create(QQmlEngine*, QJSEngine*)
    {
        Backend& instance = Instance();

        // The engine must not delete a function-local static.
        QJSEngine::setObjectOwnership(&instance, QJSEngine::CppOwnership);
        return &instance;
    }

    void Backend::Initialize(const std::shared_ptr<SettingsManager>& settings_manager,
                             const std::shared_ptr<ModuleModel>&     module_model,
                             const std::shared_ptr<TimeoutModel>&    timeout_model,
                             const std::shared_ptr<ConnectionModel>& connection_model,
                             const std::shared_ptr<SystemInfoModel>& system_info_model)
    {
        settings_manager_  = settings_manager;
        module_model_      = module_model;
        timeout_model_     = timeout_model;
        connection_model_  = connection_model;
        system_info_model_ = system_info_model;

        connection_history_   = std::make_shared<ConnectionHistoryModel>(settings_manager_, connection_model_);
        preset_model_         = std::make_shared<PresetModel>(module_model_);
        bug_report_generator_ = std::make_unique<BugReportGenerator>(system_info_model_, module_model_);
        app_settings_         = std::make_unique<AppSettingsModel>(settings_manager_);
        log_                  = std::make_unique<LoggingViewModel>(LoggingManager::Instance().GetLoggingModel());
        system_info_entries_  = std::make_unique<SystemInfoFlatModel>(system_info_model_);

        connect(system_info_model_.get(), &SystemInfoModel::IsDataValidChanged, this, &Backend::systemInfoValidChanged);

        api_model_          = std::make_shared<ApiModel>(system_info_model_);
        api_proxy_model_    = std::make_shared<ApiProxyModel>(module_model_);
        auto_connect_model_ = std::make_shared<AutoConnectModel>();

        capture_module_model_      = std::make_unique<ModuleCategoryProxyModel>(false);
        system_module_model_       = std::make_unique<ModuleCategoryProxyModel>(true);
        open_capture_module_model_ = std::make_unique<ModuleCategoryProxyModel>(false, true);
        capture_module_model_->setSourceModel(module_model_.get());
        system_module_model_->setSourceModel(module_model_.get());
        open_capture_module_model_->setSourceModel(module_model_.get());
        application_model_ = std::make_shared<ApplicationModel>(settings_manager_, connection_model_, module_model_, api_model_);
        blocklist_model_   = std::make_shared<BlocklistModel>(connection_model_, settings_manager_);

        connect(connection_model_.get(), &ConnectionModel::NetConnecting, this, [this](const QString& description) {
            connection_description_ = description;
            is_connected_           = false;
            is_connecting_          = true;
            emit connectedChanged();
        });

        connect(connection_model_.get(), &ConnectionModel::NetConnected, this, [this](const QString& description, bool) {
            connection_description_ = description;
            is_connected_           = true;
            is_connecting_          = false;
            emit connectedChanged();
            emit clientTimeoutChanged();
        });

        connect(connection_model_.get(), &ConnectionModel::NetDisconnecting, this, [this] {
            is_connected_  = false;
            is_connecting_ = false;
            emit connectedChanged();
        });

        connect(connection_model_.get(), &ConnectionModel::NetDisconnected, this, [this] {
            connection_description_.clear();
            is_connected_  = false;
            is_connecting_ = false;
            emit connectedChanged();
            emit clientTimeoutChanged();
        });

        // The same load order MainWindow::InitializeModels() used: system info feeds the
        // blocklist its platform, the blocklist feeds the application model its filter.
        connect(system_info_model_.get(), &SystemInfoModel::Loaded, blocklist_model_.get(), &BlocklistModel::OnSystemInfoModelLoaded);
        connect(blocklist_model_.get(), &BlocklistModel::Loaded, application_model_.get(), &ApplicationModel::OnBlocklistModelLoaded);

        api_proxy_model_->SetSourceApiModel(api_model_);

        connect(application_model_.get(), &ApplicationModel::ApiFilterChanged, this, &Backend::apiFilterChanged);
        connect(application_model_.get(), &ApplicationModel::AutoConnectModeChanged, this, &Backend::autoConnectModeChanged);
        connect(blocklist_model_.get(), &BlocklistModel::PlatformChanged, this, &Backend::blocklistPlatformChanged);

        // The modules' output paths are derived from the chosen application, so they are
        // re-applied whenever the application list changes under the selection.
        connect(application_model_.get(), &ApplicationModel::rowsInserted, this, &Backend::ApplyDisplayApplication);
        connect(application_model_.get(), &ApplicationModel::rowsRemoved, this, &Backend::ApplyDisplayApplication);
        connect(application_model_.get(), &ApplicationModel::modelReset, this, &Backend::ApplyDisplayApplication);
        connect(application_model_.get(), &ApplicationModel::dataChanged, this, &Backend::ApplyDisplayApplication);

        // The summary sentence names the API filter and depends on which modules are on,
        // so it has to be re-read whenever either moves.
        connect(module_model_.get(), &ModuleModel::ModuleStatusChanged, this, &Backend::apiFilterChanged);
        connect(api_proxy_model_.get(), &ApiProxyModel::SourceApiCountChanged, this, &Backend::apiFilterChanged);
        connect(this, &Backend::connectedChanged, this, &Backend::apiFilterChanged);

        api_model_->Load();
        blocklist_model_->Load();
        application_model_->Load();
        connection_history_->Load();

        // Loads the current preset, which is what gives every module its settings -- the
        // output path among them. Without it the modules keep their default-constructed
        // userdata and report an empty output path.
        preset_model_->Load();

        // The preset gives each module its raw output path; this expands it and hands the
        // module somewhere to write. Done after the preset load so the path exists.
        ApplyDisplayApplication();

        connection_model_->OnBind();

    }

    void Backend::Shutdown()
    {
        if (preset_model_ != nullptr)
        {
            preset_model_->SavePresets();
        }

        if (application_model_ != nullptr)
        {
            application_model_->Save();
        }
    }

    QString Backend::Version() const
    {
        return QStringLiteral(RDP_VERSION);
    }

    bool Backend::IsConnected() const
    {
        return is_connected_;
    }

    bool Backend::IsConnecting() const
    {
        return is_connecting_;
    }

    QString Backend::ConnectionDescription() const
    {
        return connection_description_;
    }

    QAbstractItemModel* Backend::ApplicationModelPtr() const
    {
        return application_model_.get();
    }

    QAbstractItemModel* Backend::BlocklistModelPtr() const
    {
        return blocklist_model_.get();
    }

    QAbstractItemModel* Backend::ApiModelPtr() const
    {
        return api_model_.get();
    }

    QAbstractItemModel* Backend::ModuleModelPtr() const
    {
        return module_model_.get();
    }

    QAbstractItemModel* Backend::SystemInfoModelPtr() const
    {
        return system_info_model_.get();
    }

    QAbstractItemModel* Backend::SystemInfoEntriesPtr() const
    {
        return system_info_entries_.get();
    }

    bool Backend::IsSystemInfoValid() const
    {
        return system_info_model_ != nullptr && system_info_model_->IsValid();
    }

    bool Backend::ExportSystemInfo(const QUrl& url)
    {
        if (system_info_model_ == nullptr || url.isEmpty())
        {
            return false;
        }

        return system_info_model_->ExportInfo(QDir::toNativeSeparators(url.toLocalFile()));
    }

    QAbstractItemModel* Backend::ApiFilterModelPtr() const
    {
        return api_proxy_model_.get();
    }

    QAbstractItemModel* Backend::AutoConnectModelPtr() const
    {
        return auto_connect_model_.get();
    }

    QAbstractItemModel* Backend::CaptureModuleModelPtr() const
    {
        return capture_module_model_.get();
    }

    QAbstractItemModel* Backend::SystemModuleModelPtr() const
    {
        return system_module_model_.get();
    }

    QAbstractItemModel* Backend::OpenCaptureModuleModelPtr() const
    {
        return open_capture_module_model_.get();
    }

    void Backend::CloseCaptureModule(const int row)
    {
        if (open_capture_module_model_ == nullptr || module_model_ == nullptr)
        {
            return;
        }

        // Closing a tab is how the CAPTURE view disables a feature; the tab disappears
        // because the module drops out of this proxy.
        module_model_->SetModuleEnabledAtRow(open_capture_module_model_->SourceRow(row), false);
    }

    void Backend::SetCaptureModuleEnabled(const int row, const bool is_enabled)
    {
        if (capture_module_model_ == nullptr || module_model_ == nullptr)
        {
            return;
        }

        module_model_->SetModuleEnabledAtRow(capture_module_model_->SourceRow(row), is_enabled);
    }

    int Backend::AutoConnectMode() const
    {
        return application_model_ == nullptr ? 0 : static_cast<int>(application_model_->GetAutoConnectMode());
    }

    void Backend::SetAutoConnectMode(const int mode)
    {
        if (application_model_ == nullptr || mode == AutoConnectMode())
        {
            return;
        }

        application_model_->SetAutoConnectionMode(static_cast<ApplicationAutoConnectMode>(mode));
    }

    int Backend::ApiFilterIndex() const
    {
        if (application_model_ == nullptr || api_proxy_model_ == nullptr)
        {
            return 0;
        }

        const int index = api_proxy_model_->GetIndexOf(application_model_->GetApiFilter());
        return index < 0 ? 0 : index;
    }

    void Backend::SetApiFilterIndex(const int index)
    {
        if (application_model_ == nullptr || api_proxy_model_ == nullptr)
        {
            return;
        }

        const QModelIndex model_index = api_proxy_model_->index(index, 0, {});
        if (!model_index.isValid())
        {
            return;
        }

        const auto api = static_cast<ApiModel::Api>(model_index.data(Qt::UserRole).toInt());
        if (api == application_model_->GetApiFilter())
        {
            return;
        }

        application_model_->SetApiFilter(api);
    }

    QString Backend::AutoConnectSummary() const
    {
        return application_model_ == nullptr ? QString() : application_model_->GetConnectionBehaviorDescription();
    }

    QString Backend::BlocklistPlatformName() const
    {
        if (blocklist_model_ == nullptr)
        {
            return {};
        }

        switch (blocklist_model_->GetPlatform())
        {
        case BlocklistModel::Platform::kWindows:
            return QStringLiteral("Windows");
        case BlocklistModel::Platform::kLinux:
            return QStringLiteral("Linux");
        default:
            return {};
        }
    }

    void Backend::SetDisplayApplicationRow(const int row)
    {
        display_application_row_ = row;
        ApplyDisplayApplication();
    }

    void Backend::ApplyDisplayApplication()
    {
        if (application_model_ == nullptr)
        {
            return;
        }

        // An invalid index makes the model fall back to the first application, and an empty
        // list clears the name, which is what the Qt Widgets application list did.
        application_model_->SetModuleDisplayApplication(application_model_->index(display_application_row_, 0, {}));
    }

    void Backend::AddApplication(const QString& name)
    {
        if (application_model_ != nullptr && !name.trimmed().isEmpty())
        {
            application_model_->AddApplication(name.trimmed());
        }
    }

    void Backend::AddBlockedApplication(const QString& name)
    {
        if (blocklist_model_ != nullptr && !name.trimmed().isEmpty())
        {
            blocklist_model_->AddApplication(name.trimmed());
        }
    }

    void Backend::RestoreBlocklistDefaults()
    {
        if (blocklist_model_ != nullptr)
        {
            blocklist_model_->RestoreDefaults();
        }
    }

    ConnectionHistoryModel* Backend::ConnectionHistory() const
    {
        return connection_history_.get();
    }

    AppSettingsModel* Backend::Settings() const
    {
        return app_settings_.get();
    }

    LoggingViewModel* Backend::Log() const
    {
        return log_.get();
    }

    void Backend::AttemptAutoConnection()
    {
        if (connection_history_ != nullptr)
        {
            connection_history_->AttemptAutoConnection();
        }
    }

    void Backend::AttemptNewRemoteConnection(const QString& target)
    {
        if (connection_history_ != nullptr)
        {
            connection_history_->AttemptNewRemoteConnection(target);
        }
    }

    void Backend::ConnectLocal()
    {
        if (connection_history_ != nullptr)
        {
            connection_history_->ConnectLocal();
        }
    }

    void Backend::ConnectRemote(const QString& host, int port)
    {
        if (connection_history_ != nullptr)
        {
            connection_history_->ConnectRemote(host, QString(), port);
        }
    }

    void Backend::Disconnect()
    {
        if (connection_model_ != nullptr)
        {
            connection_model_->DisconnectTool();
        }
    }

    bool Backend::IsClientTimeoutDisabled() const
    {
        return connection_model_ != nullptr && connection_model_->GetIsClientTimeoutDisabled();
    }

    void Backend::SetClientTimeoutDisabled(const bool disabled)
    {
        if (connection_model_ == nullptr || disabled == IsClientTimeoutDisabled())
        {
            return;
        }

        connection_model_->SetDisableClientTimeout(disabled);
        emit clientTimeoutChanged();
    }

    bool Backend::CanDisableClientTimeout() const
    {
        if (connection_model_ == nullptr)
        {
            return false;
        }

        // Before a connection exists the setting is still editable: it is read when the
        // router is created. Once connected, only the router's own answer counts.
        if (!connection_model_->IsConnected())
        {
            return true;
        }

        return connection_model_->CanClientTimeoutBeDisabledForCurrentConnection();
    }

    int Backend::RetryTimeoutMs() const
    {
        return timeout_model_ == nullptr ? 0 : static_cast<int>(timeout_model_->GetSavedTimeouts().retry_timeout_ms);
    }

    int Backend::CommunicationTimeoutMs() const
    {
        return timeout_model_ == nullptr ? 0 : static_cast<int>(timeout_model_->GetSavedTimeouts().communication_timeout_ms);
    }

    int Backend::ConnectionTimeoutMs() const
    {
        return timeout_model_ == nullptr ? 0 : static_cast<int>(timeout_model_->GetSavedTimeouts().connection_timeout_ms);
    }

    void Backend::SaveTimeouts(const int retry_ms, const int communication_ms, const int connection_ms)
    {
        if (timeout_model_ == nullptr)
        {
            return;
        }

        CustomDDTimeouts timeouts     = {};
        timeouts.retry_timeout_ms     = retry_ms;
        timeouts.communication_timeout_ms = communication_ms;
        timeouts.connection_timeout_ms    = connection_ms;

        timeout_model_->SetTimeouts(timeouts);
        emit timeoutsChanged();
    }

    void Backend::ResetTimeouts()
    {
        if (timeout_model_ != nullptr)
        {
            timeout_model_->ResetTimeouts();
            emit timeoutsChanged();
        }
    }

    void Backend::CopyBugReport()
    {
        if (bug_report_generator_ != nullptr)
        {
            bug_report_generator_->CopyReport();
        }
    }

    void Backend::OpenHelp()
    {
#ifdef Q_OS_MACOS
        QDir dir(QCoreApplication::applicationDirPath());
        dir.cdUp();
        dir.cd("Resources");
        const QString help_path = dir.path() + kHelpFilePath;
#else
        const QString help_path = QCoreApplication::applicationDirPath() + kHelpFilePath;
#endif

        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(help_path)))
        {
            RDP_LOG_ERROR("Failed to open help at: {}", qUtf8Printable(help_path));
        }
    }
}  // namespace rdp
