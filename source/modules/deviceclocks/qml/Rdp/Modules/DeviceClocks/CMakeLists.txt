#######################################################################################################################
### Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

# The Device Clocks module's view. Built as its own QML module so the front-end can load it
# by URL without knowing anything about the module beyond the DevToolsModule interface.

qt_add_library(rdp_module_deviceclocks STATIC)

qt_add_qml_module(rdp_module_deviceclocks
        URI Rdp.Modules.DeviceClocks
        QML_FILES
        ClockModeCard.qml
        DeviceClocksView.qml
        IMPORTS
        TARGET rdp_theme
        TARGET rdp_controls
)

devtools_target_options(rdp_module_deviceclocks)

target_link_libraries(rdp_module_deviceclocks
        PUBLIC
        Qt::Quick
        rdp_theme
        rdp_controls
)
