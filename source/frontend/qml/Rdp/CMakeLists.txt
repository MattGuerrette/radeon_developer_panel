#######################################################################################################################
### Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

add_subdirectory(Backend)
add_subdirectory(Views)

if (WIN32)
    set(RDP_WINDOWS_SRC ${RDP_FRONTEND_DIR}/rdp.rc)
endif ()

qt_add_executable(${RDP_TARGET}
        ${RDP_FRONTEND_DIR}/main.cpp
        ${RDP_FRONTEND_DIR}/resources.qrc
        ${RDP_WINDOWS_SRC}
)

qt_add_qml_module(${RDP_TARGET}
        URI Rdp
        QML_FILES
        Main.qml
        IMPORTS
        TARGET rdp_theme
        TARGET rdp_backend
        TARGET rdp_controls
        TARGET rdp_views
)

devtools_target_options(${RDP_TARGET})

set_target_properties(${RDP_TARGET} PROPERTIES
        WIN32_EXECUTABLE TRUE
)

# Set the RPATH properly on Linux
if (LINUX)
    set_target_properties(${RDP_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
endif ()

target_link_libraries(${RDP_TARGET} PRIVATE
        Qt::Quick
        Qt::QuickControls2
        rdp_theme
        rdp_themeplugin
        rdp_backend
        rdp_backendplugin
        rdp_controls
        rdp_controlsplugin
        rdp_views
        rdp_viewsplugin

        # The module views are static QML modules reached only by URL at run time, so the
        # executable has to link their plugins to pull in their resources.
        rdp_module_common
        rdp_module_commonplugin
        rdp_module_deviceclocks
        rdp_module_deviceclocksplugin
        rdp_module_profiling
        rdp_module_profilingplugin
        rdp_module_raytracing
        rdp_module_raytracingplugin
        rdp_module_memorytrace
        rdp_module_memorytraceplugin
        rdp_module_crashanalysis
        rdp_module_crashanalysisplugin
)

if (WIN32)
    # Add dependency on EnableSyncPrimitives.exe utility application
    add_dependencies(${RDP_TARGET} EnableSyncPrimitives)
endif ()

# Post-Build
add_custom_command(TARGET ${RDP_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:gpa_counters> $<TARGET_FILE_DIR:${RDP_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/scripts $<TARGET_FILE_DIR:${RDP_TARGET}>/scripts
)
if (WIN32)
    add_custom_command(TARGET ${RDP_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:EnableSyncPrimitives> $<TARGET_FILE_DIR:${RDP_TARGET}>/scripts
    )
endif ()

if (LINUX)
    # Ensure that wayland plugin integration is enabled for deployment to work
    qt_import_plugins(${RDP_TARGET} INCLUDE Qt6::QWaylandIntegrationPlugin)
endif ()

# Deploy Qt binaries for build and install
deploy_qt(${RDP_TARGET} apps QML_DIR ${RDP_FRONTEND_DIR}/qml)
