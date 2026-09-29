// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Definition for the model the Profiling QML view binds to.

#ifndef RDP_SOURCE_MODULES_PROFILING_SRC_GUI_PROFILING_CAPTURE_VIEW_MODEL_H_
#define RDP_SOURCE_MODULES_PROFILING_SRC_GUI_PROFILING_CAPTURE_VIEW_MODEL_H_

#include <memory>

#include <QStringList>
#include <QtQml/qqmlregistration.h>

#include <common/inc/global_shortcut.h>
#include <common/inc/model/capture_view_model.h>

#include "profiling_userdata_view_model.h"
#include "profiling_view_model.h"

class CurrentConnectionModel;

/// @brief The model the Profiling QML view binds to.
///
/// Turns the two view models the Qt Widgets Profiling view was wired to -- ProfilingViewModel
/// for the capture itself and ProfilingUserdataViewModel for the settings that travel to the
/// driver -- into the properties a QML view can bind to.
class ProfilingCaptureViewModel final : public CaptureViewModel
{
    Q_OBJECT
    QML_ANONYMOUS

    // -- Capture target and mode -------------------------------------------------------
    Q_PROPERTY(QAbstractItemModel* captureTargets READ CaptureTargets CONSTANT)
    Q_PROPERTY(int captureTargetIndex READ CaptureTargetIndex WRITE SetCaptureTargetIndex NOTIFY captureTargetsChanged)
    Q_PROPERTY(bool captureTargetSelectable READ IsCaptureTargetSelectable NOTIFY captureTargetsChanged)

    Q_PROPERTY(QStringList captureModes READ CaptureModes NOTIFY captureModesChanged)
    Q_PROPERTY(int captureModeIndex READ CaptureModeIndex WRITE SetCaptureModeIndex NOTIFY captureModesChanged)
    Q_PROPERTY(bool captureModeSelectable READ IsCaptureModeSelectable NOTIFY captureModesChanged)

    /// Draw and dispatch captures take a count; the other modes do not.
    Q_PROPERTY(bool renderOpCountVisible READ IsRenderOpCountVisible NOTIFY captureModesChanged)
    Q_PROPERTY(QString renderOpCountLabel READ RenderOpCountLabel NOTIFY captureModesChanged)
    Q_PROPERTY(int renderOpCount READ RenderOpCount WRITE SetRenderOpCount NOTIFY renderOpCountChanged)

    /// True from the moment a capture is asked for until it finishes, fails or is abandoned.
    Q_PROPERTY(bool capturing READ IsCapturing NOTIFY capturingChanged)

    // -- Hotkey and delay --------------------------------------------------------------
    Q_PROPERTY(QString captureShortcut READ CaptureShortcut NOTIFY shortcutChanged)
    Q_PROPERTY(bool listeningForShortcut READ IsListeningForShortcut NOTIFY shortcutChanged)
    Q_PROPERTY(bool shortcutSupported READ IsShortcutSupported CONSTANT)

    Q_PROPERTY(bool delayCaptureEnabled READ IsDelayCaptureEnabled WRITE SetDelayCaptureEnabled NOTIFY delayChanged)
    Q_PROPERTY(int captureDelayMs READ CaptureDelayMs WRITE SetCaptureDelayMs NOTIFY delayChanged)

    // -- Advanced settings -------------------------------------------------------------
    Q_PROPERTY(bool instructionTracingEnabled READ IsInstructionTracingEnabled WRITE SetInstructionTracingEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool spmCaptureEnabled READ IsSpmCaptureEnabled WRITE SetSpmCaptureEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool spmCaptureSupported READ IsSpmCaptureSupported NOTIFY settingsChanged)
    Q_PROPERTY(bool shaderInstrumentationEnabled READ IsShaderInstrumentationEnabled WRITE SetShaderInstrumentationEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool shaderInstrumentationEditable READ IsShaderInstrumentationEditable NOTIFY settingsChanged)

    Q_PROPERTY(QStringList sqttProfiles READ SqttProfiles CONSTANT)
    Q_PROPERTY(int sqttProfileIndex READ SqttProfileIndex WRITE SetSqttProfileIndex NOTIFY settingsChanged)

    Q_PROPERTY(QStringList autoCaptureModes READ AutoCaptureModes CONSTANT)
    Q_PROPERTY(int autoCaptureMode READ AutoCaptureMode WRITE SetAutoCaptureMode NOTIFY settingsChanged)
    Q_PROPERTY(int computeAutoCaptureTimeMs READ ComputeAutoCaptureTimeMs WRITE SetComputeAutoCaptureTimeMs NOTIFY settingsChanged)
    Q_PROPERTY(int dispatchStartIndex READ DispatchStartIndex WRITE SetDispatchStartIndex NOTIFY settingsChanged)
    Q_PROPERTY(int dispatchCount READ DispatchCount WRITE SetDispatchCount NOTIFY settingsChanged)
    Q_PROPERTY(int frameCaptureIndex READ FrameCaptureIndex WRITE SetFrameCaptureIndex NOTIFY settingsChanged)

    Q_PROPERTY(int frameIndexMinimum READ FrameIndexMinimum CONSTANT)
    Q_PROPERTY(int dispatchIndexMinimum READ DispatchIndexMinimum CONSTANT)

    /// Settings the driver only reads before an application connects.
    Q_PROPERTY(bool prelaunchSettingsEditable READ ArePrelaunchSettingsEditable NOTIFY settingsChanged)
    Q_PROPERTY(bool autoCaptureSettingsEditable READ AreAutoCaptureSettingsEditable NOTIFY settingsChanged)

public:
    /// @brief Constructor.
    /// @param [in] view_model The profiling view model.
    /// @param [in] userdata_view_model The profiling userdata view model.
    /// @param [in] parent The parent object.
    ProfilingCaptureViewModel(const std::shared_ptr<ProfilingViewModel>&         view_model,
                              const std::shared_ptr<ProfilingUserdataViewModel>& userdata_view_model,
                              QObject*                                           parent = nullptr);

    /// @brief Destructor.
    ~ProfilingCaptureViewModel() override;

    /// @brief Binds to the models and pulls the current state out of them.
    void Bind();

    /// @brief Gets the connections a capture can be taken from.
    /// @return The capture target model.
    [[nodiscard]] QAbstractItemModel* CaptureTargets() const;

    /// @brief Gets the selected capture target's row.
    /// @return The row, or -1 when there is no target.
    [[nodiscard]] int CaptureTargetIndex() const;

    /// @brief Selects the capture target in the given row.
    /// @param [in] index The row of the target.
    void SetCaptureTargetIndex(int index);

    /// @brief Returns whether there is a choice of capture target to make.
    /// @return true when the target can be chosen.
    [[nodiscard]] bool IsCaptureTargetSelectable() const;

    /// @brief Gets the capture modes the target supports.
    /// @return The capture mode names.
    [[nodiscard]] QStringList CaptureModes() const;

    /// @brief Gets the selected capture mode's row.
    /// @return The row, or -1 when there is no mode.
    [[nodiscard]] int CaptureModeIndex() const;

    /// @brief Selects the capture mode in the given row.
    /// @param [in] index The row of the mode.
    void SetCaptureModeIndex(int index);

    /// @brief Returns whether there is a choice of capture mode to make.
    /// @return true when the mode can be chosen.
    [[nodiscard]] bool IsCaptureModeSelectable() const;

    /// @brief Returns whether the selected mode captures a number of draws or dispatches.
    /// @return true when the render op count applies.
    [[nodiscard]] bool IsRenderOpCountVisible() const;

    /// @brief Gets the prompt for the render op count, which names draws or dispatches.
    /// @return The prompt.
    [[nodiscard]] QString RenderOpCountLabel() const;

    /// @brief Gets the number of draws or dispatches to capture.
    /// @return The render op count.
    [[nodiscard]] int RenderOpCount() const;

    /// @brief Sets the number of draws or dispatches to capture.
    /// @param [in] count The render op count.
    void SetRenderOpCount(int count);

    /// @brief Returns whether a capture is under way.
    /// @return true while capturing.
    [[nodiscard]] bool IsCapturing() const;

    /// @brief Gets the capture hotkey, as text.
    /// @return The hotkey text.
    [[nodiscard]] QString CaptureShortcut() const;

    /// @brief Returns whether the next key press will be taken as the new hotkey.
    /// @return true while listening.
    [[nodiscard]] bool IsListeningForShortcut() const;

    /// @brief Returns whether global hotkeys work on this platform.
    /// @return true when hotkeys are supported.
    [[nodiscard]] static bool IsShortcutSupported();

    /// @brief Returns whether captures wait before starting.
    /// @return true when the capture is delayed.
    [[nodiscard]] bool IsDelayCaptureEnabled() const;

    /// @brief Sets whether captures wait before starting.
    /// @param [in] enabled true to delay the capture.
    void SetDelayCaptureEnabled(bool enabled);

    /// @brief Gets how long a delayed capture waits.
    /// @return The delay, in milliseconds.
    [[nodiscard]] int CaptureDelayMs() const;

    /// @brief Sets how long a delayed capture waits.
    /// @param [in] delay_ms The delay, in milliseconds.
    void SetCaptureDelayMs(int delay_ms);

    /// @brief Returns whether instruction tracing is on.
    /// @return true when instruction tracing is enabled.
    [[nodiscard]] bool IsInstructionTracingEnabled() const;

    /// @brief Turns instruction tracing on or off.
    /// @param [in] enabled true to enable instruction tracing.
    void SetInstructionTracingEnabled(bool enabled);

    /// @brief Returns whether counter collection is on.
    /// @return true when SPM capture is enabled.
    [[nodiscard]] bool IsSpmCaptureEnabled() const;

    /// @brief Turns counter collection on or off.
    /// @param [in] enabled true to enable SPM capture.
    void SetSpmCaptureEnabled(bool enabled);

    /// @brief Returns whether the connected device can collect counters.
    /// @return true when SPM capture is supported.
    [[nodiscard]] bool IsSpmCaptureSupported() const;

    /// @brief Returns whether shader instrumentation is on.
    /// @return true when shader instrumentation is enabled.
    [[nodiscard]] bool IsShaderInstrumentationEnabled() const;

    /// @brief Turns shader instrumentation on or off.
    /// @param [in] enabled true to enable shader instrumentation.
    void SetShaderInstrumentationEnabled(bool enabled);

    /// @brief Returns whether shader instrumentation can be changed right now.
    /// @return true when the setting is editable.
    [[nodiscard]] bool IsShaderInstrumentationEditable() const;

    /// @brief Gets the names of the SQTT buffer size profiles.
    /// @return The profile names.
    [[nodiscard]] static QStringList SqttProfiles();

    /// @brief Gets the selected SQTT buffer size profile.
    /// @return The profile index.
    [[nodiscard]] int SqttProfileIndex() const;

    /// @brief Selects an SQTT buffer size profile.
    /// @param [in] index The profile index.
    void SetSqttProfileIndex(int index);

    /// @brief Gets the names of the automatic capture modes.
    /// @return The mode names.
    [[nodiscard]] static QStringList AutoCaptureModes();

    /// @brief Gets the selected automatic capture mode.
    /// @return The mode, as a devtrace::AutoCaptureMode.
    [[nodiscard]] int AutoCaptureMode() const;

    /// @brief Selects an automatic capture mode.
    /// @param [in] mode The mode, as a devtrace::AutoCaptureMode.
    void SetAutoCaptureMode(int mode);

    /// @brief Gets how long a timed automatic capture waits.
    /// @return The time, in milliseconds.
    [[nodiscard]] int ComputeAutoCaptureTimeMs() const;

    /// @brief Sets how long a timed automatic capture waits.
    /// @param [in] time_ms The time, in milliseconds.
    void SetComputeAutoCaptureTimeMs(int time_ms);

    /// @brief Gets the dispatch a range capture starts at.
    /// @return The dispatch start index.
    [[nodiscard]] int DispatchStartIndex() const;

    /// @brief Sets the dispatch a range capture starts at.
    /// @param [in] dispatch_start The dispatch start index.
    void SetDispatchStartIndex(int dispatch_start);

    /// @brief Gets how many dispatches an automatic capture takes.
    /// @return The dispatch count.
    [[nodiscard]] int DispatchCount() const;

    /// @brief Sets how many dispatches an automatic capture takes.
    /// @param [in] dispatch_count The dispatch count.
    void SetDispatchCount(int dispatch_count);

    /// @brief Gets the frame an automatic capture takes.
    /// @return The frame index.
    [[nodiscard]] int FrameCaptureIndex() const;

    /// @brief Sets the frame an automatic capture takes.
    /// @param [in] frame_index The frame index.
    void SetFrameCaptureIndex(int frame_index);

    /// @brief Gets the earliest frame that can be captured automatically.
    /// @return The minimum frame index.
    [[nodiscard]] static int FrameIndexMinimum();

    /// @brief Gets the earliest dispatch that can be captured automatically.
    /// @return The minimum dispatch index.
    [[nodiscard]] static int DispatchIndexMinimum();

    /// @brief Returns whether the settings the driver reads at launch can be changed.
    /// @return true when no application is connected.
    [[nodiscard]] bool ArePrelaunchSettingsEditable() const;

    /// @brief Returns whether the automatic capture settings can be changed.
    /// @return true when they are editable.
    [[nodiscard]] bool AreAutoCaptureSettingsEditable() const;

    /// @brief Makes the current capture mode the default for the connected application.
    Q_INVOKABLE void SetDefaultCaptureMode();

    /// @brief Takes the next key press as the new capture hotkey.
    /// @param [in] window The window to watch for the key press.
    Q_INVOKABLE void BeginShortcutCapture(QObject* window);

    /// @brief Stops taking the next key press as the new capture hotkey.
    Q_INVOKABLE void EndShortcutCapture();

    /// @brief Puts the capture hotkey back to its default.
    Q_INVOKABLE void ResetShortcut();

signals:
    /// @brief Emitted when the capture targets, or the selected one, change.
    void captureTargetsChanged();

    /// @brief Emitted when the capture modes, or the selected one, change.
    void captureModesChanged();

    /// @brief Emitted when the number of draws or dispatches to capture changes.
    void renderOpCountChanged();

    /// @brief Emitted when a capture starts or finishes.
    void capturingChanged();

    /// @brief Emitted when the capture hotkey, or whether it is being chosen, changes.
    void shortcutChanged();

    /// @brief Emitted when the capture delay changes.
    void delayChanged();

    /// @brief Emitted when any of the module's settings change.
    void settingsChanged();

protected:
    /// @brief QObject::eventFilter() override, used to read the new capture hotkey.
    ///
    /// The hotkey needs the platform's virtual key code, which Qt Quick's KeyEvent does not
    /// carry, so the key press is read from the window's QKeyEvent instead.
    /// @param [in] watched The object the event was sent to.
    /// @param [in] event The event.
    /// @return true when the event was the hotkey and should go no further.
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /// @brief Tells the userdata model whether an application is connected.
    ///
    /// The driver reads the prelaunch settings when the application starts, so they can only
    /// be changed while nothing is connected.
    void UpdatePrelaunchEditableState();

    /// @brief Applies a new capture hotkey.
    /// @param [in] shortcut The shortcut to apply.
    void ApplyShortcut(const GlobalShortcut& shortcut);

    std::shared_ptr<ProfilingViewModel>         view_model_;           ///< The profiling view model.
    std::shared_ptr<ProfilingUserdataViewModel> userdata_view_model_;  ///< The profiling userdata view model.

    CurrentConnectionModel* capture_targets_;  ///< The connections a capture can be taken from.

    std::vector<CaptureMode> capture_modes_;         ///< The capture modes the target supports.
    uint32_t                 current_capture_mode_;  ///< The selected capture mode.

    QObject*       shortcut_window_ = nullptr;  ///< The window being watched for a new hotkey.
    GlobalShortcut shortcut_;                   ///< The current capture hotkey.

    bool capturing_ = false;  ///< true while a capture is under way.

    bool should_delay_capture_ = false;  ///< true when captures wait before starting.
    int  capture_delay_ms_     = 100;    ///< How long a delayed capture waits.

    bool instruction_tracing_enabled_     = false;  ///< true when instruction tracing is on.
    bool spm_capture_enabled_             = true;   ///< true when counter collection is on.
    bool spm_capture_supported_           = true;   ///< true when the device can collect counters.
    bool shader_instrumentation_enabled_  = false;  ///< true when shader instrumentation is on.
    bool shader_instrumentation_editable_ = true;   ///< true when shader instrumentation can be changed.

    int sqtt_profile_index_ = 2;  ///< The selected SQTT buffer size profile, "Default".

    int auto_capture_mode_           = 0;  ///< The selected automatic capture mode.
    int compute_auto_capture_time_ms_ = 0;  ///< How long a timed automatic capture waits.
    int dispatch_start_index_        = 1;  ///< The dispatch a range capture starts at.
    int dispatch_count_              = 0;  ///< How many dispatches an automatic capture takes.
    int frame_capture_index_         = 0;  ///< The frame an automatic capture takes.
    int draw_count_                  = 1;  ///< How many draws a draw capture takes.

    bool prelaunch_settings_editable_    = true;  ///< true when no application is connected.
    bool auto_capture_settings_editable_ = true;  ///< true when the automatic capture settings can be changed.
};

#endif
