// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Declaration of the root object the QML front-end binds to.

#ifndef RDP_SOURCE_FRONTEND_QML_BACKEND_BACKEND_H_
#define RDP_SOURCE_FRONTEND_QML_BACKEND_BACKEND_H_

#include <memory>

#include <QAbstractItemModel>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

// Included rather than forward declared: moc has to see the complete type of a QObject*
// exposed as a Q_PROPERTY, or QML reads the property as null.
#include "logging/logging_view_model.h"
#include "models/app_settings_model.h"
#include "models/connection_history_model.h"

class QJSEngine;
class QQmlEngine;

namespace rdp
{
    class ApiModel;
    class ApiProxyModel;
    class ApplicationModel;
    class AutoConnectModel;
    class BugReportGenerator;
    class BlocklistModel;
    class ConnectionModel;
    class ModuleCategoryProxyModel;
    class ModuleModel;
    class PresetModel;
    class SettingsManager;
    class SystemInfoFlatModel;
    class SystemInfoModel;
    class TimeoutModel;

    /// @brief The single object the QML front-end binds to.
    ///
    /// Owns nothing: the models are created in main() and outlive the QML engine. Backend
    /// exists to give QML one addressable root for them, and to translate the models'
    /// C++-shaped signals into the property-change notifications QML bindings need.
    class Backend : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_SINGLETON

        Q_PROPERTY(QString version READ Version CONSTANT)
        Q_PROPERTY(bool connected READ IsConnected NOTIFY connectedChanged)
        Q_PROPERTY(bool connecting READ IsConnecting NOTIFY connectedChanged)
        Q_PROPERTY(QString connectionDescription READ ConnectionDescription NOTIFY connectedChanged)

        Q_PROPERTY(QAbstractItemModel* applicationModel READ ApplicationModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* blocklistModel READ BlocklistModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* apiModel READ ApiModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* moduleModel READ ModuleModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* systemInfoModel READ SystemInfoModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* systemInfoEntries READ SystemInfoEntriesPtr CONSTANT)
        Q_PROPERTY(bool systemInfoValid READ IsSystemInfoValid NOTIFY systemInfoValidChanged)
        Q_PROPERTY(QAbstractItemModel* apiFilterModel READ ApiFilterModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* autoConnectModel READ AutoConnectModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* captureModuleModel READ CaptureModuleModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* systemModuleModel READ SystemModuleModelPtr CONSTANT)
        Q_PROPERTY(QAbstractItemModel* openCaptureModuleModel READ OpenCaptureModuleModelPtr CONSTANT)

        Q_PROPERTY(int autoConnectMode READ AutoConnectMode WRITE SetAutoConnectMode NOTIFY autoConnectModeChanged)
        Q_PROPERTY(int apiFilterIndex READ ApiFilterIndex WRITE SetApiFilterIndex NOTIFY apiFilterChanged)
        Q_PROPERTY(QString autoConnectSummary READ AutoConnectSummary NOTIFY apiFilterChanged)
        Q_PROPERTY(QString blocklistPlatformName READ BlocklistPlatformName NOTIFY blocklistPlatformChanged)
        Q_PROPERTY(rdp::ConnectionHistoryModel* connectionHistory READ ConnectionHistory CONSTANT)
        Q_PROPERTY(rdp::AppSettingsModel* settings READ Settings CONSTANT)
        Q_PROPERTY(rdp::LoggingViewModel* log READ Log CONSTANT)

        Q_PROPERTY(bool clientTimeoutDisabled READ IsClientTimeoutDisabled WRITE SetClientTimeoutDisabled NOTIFY clientTimeoutChanged)
        Q_PROPERTY(bool canDisableClientTimeout READ CanDisableClientTimeout NOTIFY clientTimeoutChanged)

        Q_PROPERTY(int retryTimeoutMs READ RetryTimeoutMs NOTIFY timeoutsChanged)
        Q_PROPERTY(int communicationTimeoutMs READ CommunicationTimeoutMs NOTIFY timeoutsChanged)
        Q_PROPERTY(int connectionTimeoutMs READ ConnectionTimeoutMs NOTIFY timeoutsChanged)

    public:
        /// @brief Destructor.
        ~Backend() override;

        /// @brief Gets the process-wide instance.
        ///
        /// The constructor is private so that the QML engine cannot default-construct a
        /// second Backend: that one would be what QML binds to, and it would have none of
        /// the models main() put into this one.
        /// @return The instance.
        static Backend& Instance();

        /// @brief QML singleton factory. Returns Instance().
        /// @return The backend instance for the QML engine.
        static Backend* create(QQmlEngine*, QJSEngine*);

        /// @brief Gives the backend the models it exposes.
        /// @param [in] settings_manager The application settings.
        /// @param [in] module_model The module model.
        /// @param [in] timeout_model The DD timeout model.
        /// @param [in] connection_model The connection model.
        /// @param [in] system_info_model The system information model.
        void Initialize(const std::shared_ptr<SettingsManager>& settings_manager,
                        const std::shared_ptr<ModuleModel>&     module_model,
                        const std::shared_ptr<TimeoutModel>&    timeout_model,
                        const std::shared_ptr<ConnectionModel>& connection_model,
                        const std::shared_ptr<SystemInfoModel>& system_info_model);

        /// @brief Writes back everything that is only persisted on exit.
        ///
        /// The presets carry each module's settings, so they have to be saved before the
        /// modules are torn down.
        void Shutdown();

        /// @brief Gets the RDP version string shown in the window title.
        /// @return The version string.
        [[nodiscard]] QString Version() const;

        /// @brief Returns whether there is an active connection.
        /// @return true when connected.
        [[nodiscard]] bool IsConnected() const;

        /// @brief Returns whether a connection attempt is in flight.
        /// @return true while connecting.
        [[nodiscard]] bool IsConnecting() const;

        /// @brief Gets the description of the current or pending connection.
        /// @return The connection description.
        [[nodiscard]] QString ConnectionDescription() const;

        /// @brief Gets the application model.
        /// @return The application model.
        [[nodiscard]] QAbstractItemModel* ApplicationModelPtr() const;

        /// @brief Gets the blocklist model.
        /// @return The blocklist model.
        [[nodiscard]] QAbstractItemModel* BlocklistModelPtr() const;

        /// @brief Gets the API model.
        /// @return The API model.
        [[nodiscard]] QAbstractItemModel* ApiModelPtr() const;

        /// @brief Gets the module model.
        /// @return The module model.
        [[nodiscard]] QAbstractItemModel* ModuleModelPtr() const;

        /// @brief Gets the system information model.
        /// @return The system information model.
        [[nodiscard]] QAbstractItemModel* SystemInfoModelPtr() const;

        /// @brief Gets the system information as a flat list of rows.
        /// @return The flattened system information.
        [[nodiscard]] QAbstractItemModel* SystemInfoEntriesPtr() const;

        /// @brief Returns whether the system information has loaded.
        /// @return true when there is system information to show.
        [[nodiscard]] bool IsSystemInfoValid() const;

        /// @brief Writes the system information to a folder.
        /// @param [in] url The folder to write to.
        /// @return true when the export succeeded.
        Q_INVOKABLE bool ExportSystemInfo(const QUrl& url);

        /// @brief Gets the API filter model, which greys out APIs no enabled module handles.
        /// @return The API filter model.
        [[nodiscard]] QAbstractItemModel* ApiFilterModelPtr() const;

        /// @brief Gets the list of automatic-connection modes.
        /// @return The auto-connect model.
        [[nodiscard]] QAbstractItemModel* AutoConnectModelPtr() const;

        /// @brief Gets the modules shown in the CAPTURE view.
        /// @return The capture module model.
        [[nodiscard]] QAbstractItemModel* CaptureModuleModelPtr() const;

        /// @brief Gets the modules shown in the SYSTEM view.
        /// @return The system module model.
        [[nodiscard]] QAbstractItemModel* SystemModuleModelPtr() const;

        /// @brief Gets the enabled capture modules, which are the tabs in the CAPTURE view.
        /// @return The open capture module model.
        [[nodiscard]] QAbstractItemModel* OpenCaptureModuleModelPtr() const;

        /// @brief Enables or disables a module by its row in the capture module model.
        /// @param [in] row The row in the capture module model.
        /// @param [in] is_enabled true to enable the module.
        Q_INVOKABLE void SetCaptureModuleEnabled(int row, bool is_enabled);

        /// @brief Closes a CAPTURE tab, which disables the module behind it.
        /// @param [in] row The row in the open capture module model.
        Q_INVOKABLE void CloseCaptureModule(int row);

        /// @brief Gets the current automatic-connection mode.
        /// @return The mode, as an ApplicationAutoConnectMode.
        [[nodiscard]] int AutoConnectMode() const;

        /// @brief Sets the automatic-connection mode.
        /// @param [in] mode The mode, as an ApplicationAutoConnectMode.
        void SetAutoConnectMode(int mode);

        /// @brief Gets the row of the API filter in the API filter model.
        /// @return The row, or 0 when the filter is not in the list.
        [[nodiscard]] int ApiFilterIndex() const;

        /// @brief Sets the API filter from a row of the API filter model.
        /// @param [in] index The row to filter on.
        void SetApiFilterIndex(int index);

        /// @brief Gets the sentence describing what the current filters will connect to.
        /// @return The summary shown under the application filters.
        [[nodiscard]] QString AutoConnectSummary() const;

        /// @brief Gets the name of the platform the blocklist currently applies to.
        /// @return The platform name, empty when it is not yet known.
        [[nodiscard]] QString BlocklistPlatformName() const;

        /// @brief Chooses the application the module views are showing.
        ///
        /// This is what expands the $(APP_NAME) macro in each module's output path, creates
        /// that folder and points the module's stream provider at it, so a capture has
        /// somewhere to be written. Modules cannot capture until it has been called.
        /// @param [in] row The row of the application, or -1 for the first one.
        Q_INVOKABLE void SetDisplayApplicationRow(int row);

        /// @brief Adds an application to the list RDP will connect to.
        /// @param [in] name The executable name.
        Q_INVOKABLE void AddApplication(const QString& name);

        /// @brief Adds an application to the blocklist.
        /// @param [in] name The executable name.
        Q_INVOKABLE void AddBlockedApplication(const QString& name);

        /// @brief Puts the blocklist back to RDP's built-in defaults.
        Q_INVOKABLE void RestoreBlocklistDefaults();

        /// @brief Gets the connection history.
        /// @return The connection history model.
        [[nodiscard]] ConnectionHistoryModel* ConnectionHistory() const;

        /// @brief Gets the model behind the settings page.
        /// @return The settings model.
        [[nodiscard]] AppSettingsModel* Settings() const;

        /// @brief Gets the model behind the output log.
        /// @return The logging view model.
        [[nodiscard]] LoggingViewModel* Log() const;

        /// @brief Connects to the last used target, if auto-connect is enabled.
        void AttemptAutoConnection();

        /// @brief Connects to a "host" or "host:port" target given on the command line.
        /// @param [in] target The target string.
        void AttemptNewRemoteConnection(const QString& target);

        /// @brief Connects to a local target.
        Q_INVOKABLE void ConnectLocal();

        /// @brief Connects to a remote target.
        /// @param [in] host The host name or IP.
        /// @param [in] port The port.
        Q_INVOKABLE void ConnectRemote(const QString& host, int port);

        /// @brief Disconnects the current target.
        Q_INVOKABLE void Disconnect();

        /// @brief Returns whether the router's client timeout is currently switched off.
        /// @return true when the client timeout is disabled.
        [[nodiscard]] bool IsClientTimeoutDisabled() const;

        /// @brief Switches the router's client timeout on or off.
        ///
        /// Takes effect on the next connection: the router reads the setting when it is created.
        /// @param [in] disabled true to disable the client timeout.
        void SetClientTimeoutDisabled(bool disabled);

        /// @brief Returns whether the client timeout can be disabled for the current connection.
        /// @return true when the setting applies.
        [[nodiscard]] bool CanDisableClientTimeout() const;

        /// @brief Gets the saved DevDriver retry timeout.
        /// @return The retry timeout, in milliseconds.
        [[nodiscard]] int RetryTimeoutMs() const;

        /// @brief Gets the saved DevDriver communication timeout.
        /// @return The communication timeout, in milliseconds.
        [[nodiscard]] int CommunicationTimeoutMs() const;

        /// @brief Gets the saved DevDriver connection timeout.
        /// @return The connection timeout, in milliseconds.
        [[nodiscard]] int ConnectionTimeoutMs() const;

        /// @brief Saves new DevDriver timeouts. RDP restarts itself to apply them.
        /// @param [in] retry_ms The retry timeout, in milliseconds.
        /// @param [in] communication_ms The communication timeout, in milliseconds.
        /// @param [in] connection_ms The connection timeout, in milliseconds.
        Q_INVOKABLE void SaveTimeouts(int retry_ms, int communication_ms, int connection_ms);

        /// @brief Puts the DevDriver timeouts back to their defaults.
        Q_INVOKABLE void ResetTimeouts();

        /// @brief Copies a bug report describing the current system to the clipboard.
        Q_INVOKABLE void CopyBugReport();

        /// @brief Opens the bundled help documentation in the user's browser.
        Q_INVOKABLE void OpenHelp();

    signals:
        /// @brief Emitted whenever the connection state changes.
        ///
        /// QML-facing signals are lowerCamelCase: QML derives the handler name by
        /// lowercasing the first letter after "on", so a PascalCase signal never binds.
        void connectedChanged();

        /// @brief Emitted when the client timeout setting, or whether it applies, changes.
        void clientTimeoutChanged();

        /// @brief Emitted when the saved DevDriver timeouts change.
        void timeoutsChanged();

        /// @brief Emitted when the automatic-connection mode changes.
        void autoConnectModeChanged();

        /// @brief Emitted when the API filter, or the set of usable APIs, changes.
        void apiFilterChanged();

        /// @brief Emitted when the platform the blocklist applies to changes.
        void blocklistPlatformChanged();

        /// @brief Emitted when the system information becomes available or goes away.
        void systemInfoValidChanged();

    private:
        /// @brief Constructor.
        Backend();

        std::shared_ptr<SettingsManager> settings_manager_;   ///< The application settings.
        std::shared_ptr<ModuleModel>     module_model_;       ///< The module model.
        std::shared_ptr<TimeoutModel>    timeout_model_;      ///< The DD timeout model.
        std::shared_ptr<ConnectionModel> connection_model_;   ///< The connection model.
        std::shared_ptr<SystemInfoModel> system_info_model_;  ///< The system information model.

        std::shared_ptr<ConnectionHistoryModel> connection_history_;  ///< The user's connection history.
        std::shared_ptr<PresetModel>            preset_model_;        ///< The presets, which carry each module's settings.
        std::unique_ptr<BugReportGenerator>     bug_report_generator_;  ///< Produces bug reports from the system information.
        std::unique_ptr<AppSettingsModel>       app_settings_;          ///< The model behind the settings page.
        std::unique_ptr<LoggingViewModel>       log_;                   ///< The model behind the output log.
        std::unique_ptr<SystemInfoFlatModel>    system_info_entries_;   ///< The system information as a flat list.

        std::shared_ptr<ApiModel>         api_model_;          ///< The API model.
        std::shared_ptr<ApiProxyModel>    api_proxy_model_;    ///< The API model as filtered by the enabled modules.
        std::shared_ptr<AutoConnectModel> auto_connect_model_; ///< The list of automatic-connection modes.

        std::unique_ptr<ModuleCategoryProxyModel> capture_module_model_;  ///< The modules shown in the CAPTURE view.
        std::unique_ptr<ModuleCategoryProxyModel> system_module_model_;   ///< The modules shown in the SYSTEM view.
        std::unique_ptr<ModuleCategoryProxyModel> open_capture_module_model_;  ///< The enabled capture modules, one per CAPTURE tab.
        std::shared_ptr<ApplicationModel> application_model_;  ///< The application model.
        std::shared_ptr<BlocklistModel>   blocklist_model_;    ///< The blocklist model.

        /// @brief Re-applies the chosen application to the modules.
        void ApplyDisplayApplication();

        int display_application_row_ = -1;  ///< The row of the application the module views show.

        QString connection_description_;     ///< Description of the current or pending connection.
        bool    is_connected_  = false;      ///< true while a connection is established.
        bool    is_connecting_ = false;      ///< true while a connection attempt is in flight.
    };
}  // namespace rdp

#endif
