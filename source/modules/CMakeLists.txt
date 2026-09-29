#######################################################################################################################
### Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

# Add current directory to include path to support #include <common/inc/*.h>
include_directories(".")

# The QML modules below declare C++ types, which needs moc.
set(CMAKE_AUTOMOC ON)

### Shared QML #######################################################################################################
# Rdp.Theme, Rdp.Controls and Rdp.Modules.Common are used by the front-end and by every
# module's view, so they live under source/modules: the modules are configured before the
# front-end and cannot depend on anything it defines. They come first because each module's
# QML module imports them by target.
add_subdirectory(common/qml/Rdp/Theme)
add_subdirectory(common/qml/Rdp/Controls)
add_subdirectory(common/qml/Rdp/Modules/Common)

### Common Shared Code ################################################################################################
add_subdirectory(common)

### Device Clocks Module ##############################################################################################
add_subdirectory(deviceclocks)

### Profiling Module ##################################################################################################
add_subdirectory(profiling)

### Raytracing Module #################################################################################################
add_subdirectory(raytracing)

### Memory Trace Module ###############################################################################################
add_subdirectory(memorytrace)

### Crash Analysis Module ###############################################################################################
add_subdirectory(crashanalysis)

