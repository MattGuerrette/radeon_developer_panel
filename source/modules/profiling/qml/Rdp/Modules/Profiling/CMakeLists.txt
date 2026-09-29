#######################################################################################################################
### Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
### uthor AMD Developer Tools Team
#######################################################################################################################

# The Profiling module's view. Built as its own QML module so the front-end can load it by
# URL without knowing anything about the module beyond the DevToolsModule interface.

qt_add_library(rdp_module_profiling STATIC)

qt_add_qml_module(rdp_module_profiling
        URI Rdp.Modules.Profiling
        QML_FILES
        ProfilingView.qml
        IMPORTS
        TARGET rdp_theme
        TARGET rdp_controls
        TARGET rdp_module_common
)

devtools_target_options(rdp_module_profiling)

target_link_libraries(rdp_module_profiling
        PUBLIC
        Qt::Quick
        rdp_theme
        rdp_controls
        rdp_module_common
)
