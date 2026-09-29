#######################################################################################################################
### Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

# The C++ half of the front-end: the models that talk to devdriver, plus the Backend
# object that publishes them to QML. Built as a library so the QML view modules can
# depend on it without the executable depending on them.

qt_add_library(rdp_backend STATIC)

set(RDP_BACKEND_SOURCE
        backend.cpp

        ${RDP_FRONTEND_DIR}/application.cpp
        ${RDP_FRONTEND_DIR}/settings_manager.cpp
        ${RDP_FRONTEND_DIR}/utilities.cpp
        ${RDP_FRONTEND_DIR}/bug/bug_report_generator.cpp
        ${RDP_FRONTEND_DIR}/crash/crash_handler.cpp
        ${RDP_FRONTEND_DIR}/logging/logging_manager.cpp
        ${RDP_FRONTEND_DIR}/logging/logging_model.cpp
        ${RDP_FRONTEND_DIR}/logging/logging_view_model.cpp
        ${RDP_FRONTEND_DIR}/models/api_model.cpp
        ${RDP_FRONTEND_DIR}/models/app_settings_model.cpp
        ${RDP_FRONTEND_DIR}/models/api_proxy_model.cpp
        ${RDP_FRONTEND_DIR}/models/application_model.cpp
        ${RDP_FRONTEND_DIR}/models/application_proxy_model.cpp
        ${RDP_FRONTEND_DIR}/models/blocklist_model.cpp
        ${RDP_FRONTEND_DIR}/models/connection_history_model.cpp
        ${RDP_FRONTEND_DIR}/models/connection_model.cpp
        ${RDP_FRONTEND_DIR}/models/module_category_proxy_model.cpp
        ${RDP_FRONTEND_DIR}/models/module_model.cpp
        ${RDP_FRONTEND_DIR}/models/preset_model.cpp
        ${RDP_FRONTEND_DIR}/models/system_info_flat_model.cpp
        ${RDP_FRONTEND_DIR}/models/system_info_model.cpp
        ${RDP_FRONTEND_DIR}/models/timeout_model.cpp
        ${RDP_FRONTEND_DIR}/models/sidebar/auto_connect_model.cpp
)

set(RDP_BACKEND_INCLUDE
        backend.h

        ${RDP_FRONTEND_DIR}/application.h
        ${RDP_FRONTEND_DIR}/definitions.h
        ${RDP_FRONTEND_DIR}/settings_manager.h
        ${RDP_FRONTEND_DIR}/tool_wrapper_types.h
        ${RDP_FRONTEND_DIR}/utilities.h
        ${RDP_FRONTEND_DIR}/bug/bug_report.h
        ${RDP_FRONTEND_DIR}/bug/bug_report_generator.h
        ${RDP_FRONTEND_DIR}/crash/crash_factory.h
        ${RDP_FRONTEND_DIR}/crash/crash_handler.h
        ${RDP_FRONTEND_DIR}/crash/crash_notify.h
        ${RDP_FRONTEND_DIR}/logging/available_logging_filters_model.h
        ${RDP_FRONTEND_DIR}/logging/logging_manager.h
        ${RDP_FRONTEND_DIR}/logging/logging_model.h
        ${RDP_FRONTEND_DIR}/logging/logging_view_model.h
        ${RDP_FRONTEND_DIR}/models/api_model.h
        ${RDP_FRONTEND_DIR}/models/app_settings_model.h
        ${RDP_FRONTEND_DIR}/models/api_proxy_model.h
        ${RDP_FRONTEND_DIR}/models/application_model.h
        ${RDP_FRONTEND_DIR}/models/application_proxy_model.h
        ${RDP_FRONTEND_DIR}/models/blocklist_model.h
        ${RDP_FRONTEND_DIR}/models/connection_history_model.h
        ${RDP_FRONTEND_DIR}/models/connection_model.h
        ${RDP_FRONTEND_DIR}/models/module_category_proxy_model.h
        ${RDP_FRONTEND_DIR}/models/module_model.h
        ${RDP_FRONTEND_DIR}/models/preset_model.h
        ${RDP_FRONTEND_DIR}/models/system_info_flat_model.h
        ${RDP_FRONTEND_DIR}/models/system_info_model.h
        ${RDP_FRONTEND_DIR}/models/timeout_model.h
        ${RDP_FRONTEND_DIR}/models/sidebar/auto_connect_model.h
)

if (WIN32)
    list(APPEND RDP_BACKEND_SOURCE
            ${RDP_FRONTEND_DIR}/crash/win/crash_notify_win.cpp
            ${RDP_FRONTEND_DIR}/crash/win/crash_handler_win.cpp)
    list(APPEND RDP_BACKEND_INCLUDE
            ${RDP_FRONTEND_DIR}/crash/win/crash_notify_win.h
            ${RDP_FRONTEND_DIR}/crash/win/crash_handler_win.h)
endif ()

qt_add_qml_module(rdp_backend
        URI Rdp.Backend
        SOURCES
        ${RDP_BACKEND_SOURCE}
        ${RDP_BACKEND_INCLUDE}
)

devtools_target_options(rdp_backend)

# The generated qmltyperegistrations.cpp includes each registered type's header by its
# bare file name, so every directory holding one has to be on the include path.
target_include_directories(rdp_backend PUBLIC
        ${RDP_FRONTEND_DIR}
        ${RDP_FRONTEND_DIR}/models
        ${RDP_FRONTEND_DIR}/logging
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${RDP_FRONTEND_DIR}/..
        ${RDP_FRONTEND_DIR}/../modules
        ${RDP_FRONTEND_DIR}/../modules/common/inc
        ${RDP_FRONTEND_DIR}/../api
        ${LIBDEVTRACE_DIR})
target_include_directories(rdp_backend SYSTEM PUBLIC ${JSON_LIB_DIR})

target_link_libraries(rdp_backend PUBLIC
        rdp_controls
        general
        dd_tools
        ddRouter
        ddNet
        QtCustomWidgets
        QtUtils
        MercuryModuleExt
        system_info
        LibDevTrace
        DevToolsMemoryTraceModule
        DevToolsProfilingModule
        DevToolsRaytracingModule
        DevToolsCrashAnalysisModule
        DevToolsDeviceClocksModule
        SystemTraceModuleStatic
        ddRouterUtils
        ddSiphonModule
        Qt::Quick
        Qt::Widgets
        Qt::Network
)
