#######################################################################################################################
### Copyright (c) 2019-2026 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

cmake_policy(SET CMP0087 NEW)
project(RadeonDeveloperPanel)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(RDP_TARGET ${PROJECT_NAME})
set(RDP_FRONTEND_DIR ${CMAKE_CURRENT_SOURCE_DIR})

qt_standard_project_setup()

# Defines the executable, its Rdp QML module, and the Rdp.Theme / Rdp.Backend /
# Rdp.Controls / Rdp.Views modules it is built from. Everything that has to be
# attached to the executable target lives there, since add_custom_command(TARGET)
# only works in the directory that created the target.
add_subdirectory(qml/Rdp)

install(TARGETS ${RDP_TARGET}
        RUNTIME DESTINATION . COMPONENT apps
        BUNDLE DESTINATION . COMPONENT apps
        ARCHIVE DESTINATION lib/ COMPONENT apps
        LIBRARY DESTINATION lib/ COMPONENT apps)

# GPA library - Use the library path exported by spm_db

if (WIN32)
    if (NOT CMAKE_BUILD_TYPE MATCHES "Release")
        install(FILES $<TARGET_PDB_FILE:${RDP_TARGET}> DESTINATION . OPTIONAL COMPONENT apps)
    endif ()
    install(FILES $<TARGET_FILE:gpa_counters> DESTINATION . COMPONENT apps)
elseif (LINUX)
    install(FILES $<TARGET_FILE:gpa_counters> DESTINATION lib/ COMPONENT apps)
endif ()

if (LINUX)
    file(GLOB KERNEL_TRACING_LIBS
            ${EXTERNAL_DIR}/libtraceevent/lib/libtraceevent.so.1
            ${EXTERNAL_DIR}/libtracefs/lib/libtracefs.so.1)
    install(FILES ${KERNEL_TRACING_LIBS} DESTINATION lib/ COMPONENT apps)
endif ()

source_group("Generated"
        ../version.h
)
