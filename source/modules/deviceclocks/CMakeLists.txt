#######################################################################################################################
### Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

set(DEVTOOLS_MODULE_FOLDER "DevTools Modules/Device Clocks")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

### QML view ##########################################################################################################
add_subdirectory(qml/Rdp/Modules/DeviceClocks)

### Module ############################################################################################################
add_library(DevToolsDeviceClocksModule STATIC)
devtools_target_options(DevToolsDeviceClocksModule)
target_link_libraries(DevToolsDeviceClocksModule
        PUBLIC
        ddModule
        PRIVATE
        dd_tools
        ddCommon
        ddModuleUtil
        MercuryModuleExt
        system_info
        DevToolsCommonStatic
        QtCustomWidgets
        QtUtils
        LibDevTrace
        rdp_controls
        Qt::Quick
)

target_include_directories(DevToolsDeviceClocksModule
        PRIVATE
        src
        ${EXTERNAL_DIR}
        ${LIBDEVTRACE_DIR}
        PUBLIC
        inc
)

target_sources(DevToolsDeviceClocksModule PRIVATE
        src/gui/clock_mode_model.h
        src/gui/clock_mode_model.cpp
        src/gui/device_clocks_gpu_model.h
        src/gui/device_clocks_gpu_model.cpp
        src/gui/device_clocks_model.h
        src/gui/device_clocks_model.cpp
        src/gui/device_clocks_view_model.h
        src/gui/device_clocks_view_model.cpp
        src/DeviceClocks.cpp
        PUBLIC
        inc/DeviceClocks.h
)
