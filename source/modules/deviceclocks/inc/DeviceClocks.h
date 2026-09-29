// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief DeviceClocks module config declarations

#ifndef RDP_MODULES_CRASH_ANALYSIS_INC_DEVICE_CLOCKS_H_
#define RDP_MODULES_CRASH_ANALYSIS_INC_DEVICE_CLOCKS_H_

#include <memory>

#include <dd_modules_api.h>

#include <client_connection_manager.h>

#include <common/inc/api/dev_tools_module.h>

/// @brief DevTools Device Clocks gui module interface.
class DeviceClocksDevToolsModule final : public DevToolsModule
{
public:
    /// @brief Default constructor.
    /// @param [in] api_registry The API registry to use for this module
    explicit DeviceClocksDevToolsModule(DDApiRegistry* api_registry);

    /// @brief Destructor. Out of line so the view model only has to be complete in the .cpp.
    ~DeviceClocksDevToolsModule() override;

protected:
    /// @brief Initializes the module.
    /// @param [in] tool_settings The tool settings.
    /// @return DD_RESULT_SUCCESS if the module initialization was successful.
    DD_RESULT Initialize(QSettings* tool_settings) override;

public:
    /// @brief Gets the QML component that renders the device clocks view.
    /// @return The URL of the module's QML view.
    [[nodiscard]] QUrl GetQmlViewUrl() const override;

    /// @brief Gets the view model the device clocks view binds to.
    /// @return The module's view model.
    [[nodiscard]] QObject* GetViewModel() const override;

    /// @brief Returns the module category.
    /// @return ModuleCategory::kSystem since device clocks operates system-wide.
    [[nodiscard]] ModuleCategory GetModuleCategory() const override;

private:
    std::shared_ptr<class DeviceClocksModel>     clocks_model_;  ///< The model that talks to the device clocks manager.
    std::unique_ptr<class DeviceClocksViewModel> view_model_;    ///< The model the QML view binds to.
};

DD_DECLARE_MODULE_LOAD_API(DeviceClocks);

#endif
