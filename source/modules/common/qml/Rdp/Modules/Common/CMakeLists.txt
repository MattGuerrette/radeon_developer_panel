#######################################################################################################################
### Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

# The capture UI shared by the Profiling, Memory Trace, Raytracing and Crash Analysis
# module views: the QML counterpart of the BaseUtilityView / SplitClientView pair the
# Qt Widgets front-end shared between them.

qt_add_library(rdp_module_common STATIC)

qt_add_qml_module(rdp_module_common
        URI Rdp.Modules.Common
        QML_FILES
        CaptureModuleView.qml
        CaptureStatusHeader.qml
        ShortcutField.qml
        IMPORTS
        TARGET rdp_theme
        TARGET rdp_controls
)

devtools_target_options(rdp_module_common)

target_link_libraries(rdp_module_common
        PUBLIC
        Qt::Quick
        rdp_theme
        rdp_controls
)
