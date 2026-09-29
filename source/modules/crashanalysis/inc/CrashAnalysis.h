// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Crash Analysis module config declarations

#ifndef RDP_MODULES_CRASH_ANALYSIS_INC_CRASH_ANALYSIS_H_
#define RDP_MODULES_CRASH_ANALYSIS_INC_CRASH_ANALYSIS_H_

#include <dd_modules_api.h>

#include <common/inc/api/dev_tools_module.h>

namespace devtrace
{
    class RgdTraceSource;
}

/// @brief DevTools Crash Analysis gui module interface.
class CrashAnalysisDevToolsModule final : public DevToolsModule
{
public:
    /// @brief Default constructor.
    /// @param [in] api_registry The API registry to use for this module
    explicit CrashAnalysisDevToolsModule(DDApiRegistry* api_registry);

    /// @brief Destructor. Out of line so the view model only has to be complete in the .cpp.
    ~CrashAnalysisDevToolsModule() override;

protected:
    DD_RESULT Initialize(QSettings* tool_settings) override;
    bool      Enable() override;
    bool      Disable() override;

public:
    /// @brief Gets the QML component that renders this module's view.
    /// @return The URL of the module's QML view.
    [[nodiscard]] QUrl GetQmlViewUrl() const override;

    /// @brief Gets the view model the module's QML view binds to.
    /// @return The module's view model.
    [[nodiscard]] QObject* GetViewModel() const override;
    [[nodiscard]] bool     IsCompatibleWithApi(devtrace::Api api) const override;
    [[nodiscard]] bool     IsCompatibleWithModuleNamed(const std::string& module_name) const override;

private:
    std::unique_ptr<class CaptureViewModel> qml_view_model_;  ///< The model the QML view binds to.

    std::shared_ptr<class CrashAnalysisUserdataViewModel> userdata_view_model_;   ///< Userdata view model for the module.
    std::shared_ptr<devtrace::RgdTraceSource>             trace_source_;          ///< The trace source.
    std::shared_ptr<class CrashAnalysisViewModel>         crash_analysis_model_;  ///< The crash analysis view model.
};

DD_DECLARE_MODULE_LOAD_API(CrashAnalysis);

#endif
