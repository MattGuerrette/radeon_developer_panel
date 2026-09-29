// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief RDP main entrypoint

#include <memory>

#include <QAbstractEventDispatcher>
#include <QCommandLineParser>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>

#include <common/inc/global_shortcut_manager.h>
#include <common/inc/single_application_instance.h>

#ifdef Q_OS_WIN
#include <common/inc/system_keyboard_hook.h>
#endif

#include "crash/crash_factory.h"
#include "crash/crash_notify.h"
#include "definitions.h"
#include "logging/logging_manager.h"
#include "models/connection_model.h"
#include "models/module_model.h"
#include "models/timeout_model.h"
#include "qml/Rdp/Backend/backend.h"
#include "settings_manager.h"
#include "utilities.h"

namespace rdp
{
    /// @brief Defines a single RadeonDevelopPanel application instance
    class Instance final : public SingleApplicationInstance
    {
    public:
        /// @brief Constructor
        /// @param [in] argc Argument count
        /// @param [in] argv Argument values
        Instance(int& argc, char** argv)
            : SingleApplicationInstance(argc, argv, kRadeonDeveloperPanelGuid)
        {
            installEventFilter(this);
        }

        /// @brief Destructor
        ~Instance() Q_DECL_OVERRIDE = default;

        bool Run()
        {
            setWindowIcon(QIcon(":/RDP_Icon.ico"));
#ifdef _WIN32
            SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#endif

            // Fusion is the only built-in Qt Quick Controls style that honours a custom
            // palette on every control, which is what Rdp.Theme relies on.
            QQuickStyle::setStyle(QStringLiteral("Fusion"));
            util::SetCurrentThreadName("Qt UI");

            LoggingManager::Instance().Initialize();

            const std::unique_ptr<GlobalShortcutNativeEventFilter> native_event_filter(new GlobalShortcutNativeEventFilter([](const QString& failure_reason) {
                RDP_LOG_ERROR("Error registering global shortcut event handler. Reason: {}", failure_reason.toStdString().c_str());
            }));

            QAbstractEventDispatcher::instance()->installNativeEventFilter(native_event_filter.get());

            // Parse command-line arguments
            QCommandLineParser parser;
            parser.setApplicationDescription("AMD Internal Developer Toolkit");
            const auto help    = parser.addHelpOption();
            const auto version = parser.addVersionOption();
            parser.addOption({{"dm", "dynamic-module"}, "Initializes <Dynamic Module> upon load", "Dynamic Module"});
            parser.addOption({"no-builtins", "Disables all builtin modules"});
            parser.addOption({{"c", "connect"}, "Connects automatically to the given target", "Target"});
            parser.parse(arguments());

            bool result        = false;
            bool needs_restart = false;

            // Initialize models / managers

            auto settings_manager = std::make_shared<SettingsManager>();
            auto module_model     = std::make_shared<ModuleModel>(settings_manager);

            auto       timeout_model     = std::make_shared<TimeoutModel>(settings_manager);
            auto       connection_model  = std::make_shared<ConnectionModel>(module_model, settings_manager, timeout_model);
            const auto system_info_model = std::make_shared<SystemInfoModel>(connection_model);

            const auto crash_notifier = CrashNotifierFactory::Create();
            const auto crash_handler  = CrashHandlerFactory::Create(system_info_model);

            if (crash_notifier != nullptr)
            {
                crash_notifier->Register();
                crash_notifier->SetCrashHandler([&](const CrashInfo& info) { crash_handler->Handle(info); });
            }

            // Handle built-in cmd-line arguments
            if (parser.isSet(help))
            {
                parser.showHelp();
            }

            if (parser.isSet(version))
            {
                parser.showVersion();
            }

            settings_manager->LoadSettings();

            if (connection_model->CreateTool())
            {
                if (connection_model->LoadModules())
                {
#ifdef Q_OS_WIN
                    connect(&SystemKeyboardHook::GetInstance(), &SystemKeyboardHook::OnRegisterFailed, [&](const DWORD error_code) {
                        RDP_LOG_ERROR("Error registering system keyboard hook. Error code: %lu", error_code);
                    });

                    SystemKeyboardHook::GetInstance().Connect();
#endif

                    // The single object the QML front-end binds to. It has to be filled in
                    // before the engine resolves Rdp.Backend, since QML binds to it directly.
                    Backend& backend = Backend::Instance();
                    backend.Initialize(settings_manager, module_model, timeout_model, connection_model, system_info_model);

                    QQmlApplicationEngine engine;
                    connect(
                        &engine,
                        &QQmlApplicationEngine::objectCreationFailed,
                        this,
                        [] { QCoreApplication::exit(-1); },
                        Qt::QueuedConnection);

                    engine.loadFromModule("Rdp", "Main");

                    if (engine.rootObjects().isEmpty())
                    {
                        RDP_LOG_ERROR("Failed to load the QML front-end.");
                        return false;
                    }

                    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());

                    connect(timeout_model.get(), &TimeoutModel::TimeoutsChanged, [&] {
                        needs_restart = true;
                        if (window != nullptr)
                        {
                            window->close();
                        }
                    });

                    connect(this, &SingleApplicationInstance::AppInstanceStarted, this, [window] { BringToForeground(window); });

                    // If the target parameter is used, try to connect to the target.
                    // Otherwise, try and connect to the last connection we used.
                    // If we fail to find a last connection to use,
                    //      this will silently do nothing and let the app start.
                    // If we find a connection to use and fail to connect,
                    //      this will have gone through the usual channels for a connection with signals
                    //      and produce a message overlay on failure to connect.
                    // This call is predicated on a checkbox visible on the CONNECTION page.

                    if (parser.isSet("c"))
                    {
                        backend.AttemptNewRemoteConnection(parser.value("c"));
                    }
                    else
                    {
                        backend.AttemptAutoConnection();
                    }

                    // Run the actual application and invoke Qt event handler
                    result = exec() == 0;

#ifdef Q_OS_WIN
                    SystemKeyboardHook::GetInstance().Disconnect();
#endif

                    backend.Shutdown();
                    settings_manager->SaveSettings();

                    connection_model->ShutDown();
                }
            }

            if (needs_restart && !arguments().isEmpty())
            {
                QProcess::startDetached(arguments()[0], arguments());
            }

            return result;
        }

    private:
        /// @brief Raises the application window above the other windows on the desktop.
        /// @param [in] window The main window.
        static void BringToForeground(QQuickWindow* window)
        {
            if (window == nullptr)
            {
                return;
            }

            if (window->windowState() & Qt::WindowMinimized)
            {
                window->showNormal();
            }

            window->raise();
            window->requestActivate();
        }
    };
}  // namespace rdp

int main(int argc, char* argv[])
{
    std::unique_ptr<rdp::Instance> instance;
    instance.reset(new (std::nothrow) rdp::Instance(argc, argv));

    if (instance == nullptr || instance->IsAnotherInstanceRunning())
    {
        instance.reset();
        return -1;
    }

    return instance->Run() ? 0 : -1;
}
