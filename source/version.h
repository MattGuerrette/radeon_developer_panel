//=============================================================================
// Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Radeon Developer Panel version header
//=============================================================================

#ifndef RDP_FRONTEND_VERSION_H_
#define RDP_FRONTEND_VERSION_H_

#define STRINGIFY_MACRO_(a) #a
#define STRINGIFY_MACRO(a) STRINGIFY_MACRO_(a)
#define STRINGIFY_VERSION(major, minor, patch, build) STRINGIFY_MACRO(v) STRINGIFY_MACRO(major) "." STRINGIFY_MACRO(minor) "." STRINGIFY_MACRO(patch) "." STRINGIFY_MACRO(build)

#define RDP_MAJOR_VERSION 3  ///< Major version number.
#define RDP_MINOR_VERSION 5  ///< Minor version number.
#define RDP_PATCH_NUMBER  0   ///< Patch number.
#define RDP_BUILD_NUMBER  0   ///< Build number.
#define RDP_BUILD_SUFFIX     ///< Build suffix
#define RDP_BUILD_DATE_STRING "09/07/2026"
#define RDP_BUILD_YEAR "2026"
#define RDP_VERSION STRINGIFY_VERSION(RDP_MAJOR_VERSION, RDP_MINOR_VERSION, RDP_PATCH_NUMBER, RDP_BUILD_NUMBER)
#define RDP_TITLE "Radeon Developer Panel " RDP_VERSION RDP_BUILD_SUFFIX ///< Title

#endif
