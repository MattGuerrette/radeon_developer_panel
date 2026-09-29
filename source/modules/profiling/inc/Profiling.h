// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Profiling module config declarations

#ifndef RDP_SOURCE_MODULES_PROFILING_INC_PROFILING_H_
#define RDP_SOURCE_MODULES_PROFILING_INC_PROFILING_H_

#include <dd_modules_api.h>

#include <common/inc/api/dev_tools_module.h>
#include <rgp_trace_source.h>

/// @brief DevTools Profiling GUI module interface.
class ProfilingDevToolsModule final : public DevToolsModule
{
public:
    /// @brief Default constructor.
    /// @param [in] api_registry The API registry to use for this module
    explicit ProfilingDevToolsModule(DDApiRegistry* api_registry);

    /// @brief Destructor. Out of line so the view model only has to be complete in the .cpp.
    ~ProfilingDevToolsModule() override;

protected:
    /// @brief Initializes the module.
    /// @param [in] tool_settings The tool settings.
    /// @return DD_RESULT_SUCCESS if the module initialization was successful.
    DD_RESULT Initialize(class QSettings* tool_settings) override;

    bool Enable() override;
    bool Disable() override;

public:
    /// @brief Creates a new profiling view.
    /// @return A new profiling view.
    /// @brief Gets the QML component that renders this module's view.
    /// @return The URL of the module's QML view.
    [[nodiscard]] QUrl GetQmlViewUrl() const override;

    /// @brief Gets the view model the module's QML view binds to.
    /// @return The module's view model.
    [[nodiscard]] QObject* GetViewModel() const override;

    /// @brief Returns whether Profiling is compatible with the given API.
    /// @param [in] api The API to check module compatibility against.
    /// @return true if Profiling is compatible with the API, false otherwise.
    [[nodiscard]] bool IsCompatibleWithApi(devtrace::Api api) const override;

private:
    std::unique_ptr<class CaptureViewModel> qml_view_model_;  ///< The model the QML view binds to.

    std::shared_ptr<devtrace::RgpTraceSource>         trace_source_;         ///< Trace source.
    std::shared_ptr<class ProfilingViewModel>         view_model_;           // View model for module.
    std::shared_ptr<class ProfilingUserdataViewModel> userdata_view_model_;  ///< Userdata view model for the module.
};

DD_DECLARE_MODULE_LOAD_API(Profiling);

#endif
