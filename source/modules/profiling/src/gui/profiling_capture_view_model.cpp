// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the model the Profiling QML view binds to.

#include "profiling_capture_view_model.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QKeySequence>

#include <rgp_trace_source.h>

#include <common/inc/global_shortcut_manager.h>
#include <common/inc/model/current_connection_model.h>

#include "notifications.h"

namespace
{
    /// The SQTT buffer size profiles, in devtrace::SqttBufferSizeProfiles order.
    const QStringList kSqttProfileNames = {"Minimum", "Low", "Default", "High", "Maximum"};

    /// The automatic capture modes, in devtrace::AutoCaptureMode order.
    const QStringList kAutoCaptureModeNames = {"None", "Frame index", "Dispatch range", "Dispatch timer"};

    constexpr int kFrameIndexMinimum    = 5;  ///< The earliest frame that can be captured automatically.
    constexpr int kDispatchIndexMinimum = 1;  ///< The earliest dispatch that can be captured automatically.

    /// Modifier and navigation keys cannot be a hotkey on their own.
    const QList<int> kIgnoredShortcutKeys = {Qt::Key_Control, Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Meta, Qt::Key_unknown, Qt::Key_Enter, Qt::Key_Return};
}  // namespace

ProfilingCaptureViewModel::ProfilingCaptureViewModel(const std::shared_ptr<ProfilingViewModel>&         view_model,
                                                     const std::shared_ptr<ProfilingUserdataViewModel>& userdata_view_model,
                                                     QObject*                                           parent)
    : CaptureViewModel(view_model, userdata_view_model, [view_model] { view_model->RequestBeginTrace(); }, parent)
    , view_model_(view_model)
    , userdata_view_model_(userdata_view_model)
    , capture_targets_(new CurrentConnectionModel(this))
    , current_capture_mode_(0)
{
    shortcut_ = ProfilingUserdataViewModel::GetDefaultShortcut();
}

ProfilingCaptureViewModel::~ProfilingCaptureViewModel()
{
    EndShortcutCapture();
}

void ProfilingCaptureViewModel::Bind()
{
    // -- Capture target and mode -------------------------------------------------------
    connect(view_model_.get(), &TraceSourceViewModel::CurrentConnectionsChanged, this, [this](const CurrentConnections& connections) {
        capture_targets_->OnCurrentConnectionsChanged(connections);
        emit captureTargetsChanged();
        UpdatePrelaunchEditableState();
    });

    // A client that disconnects out of the kDone state leaves the connections map already
    // empty, so CurrentConnectionsChanged does not fire for it and the process text is the
    // only thing that moves.
    connect(view_model_.get(), &TraceSourceViewModel::ConnectedProcessTextChanged, this, [this](const QString&) { UpdatePrelaunchEditableState(); });

    connect(view_model_.get(), &TraceSourceViewModel::CaptureTargetChanged, this, [this](uint16_t) { emit captureTargetsChanged(); });

    connect(view_model_.get(), &ProfilingViewModel::AvailableCaptureModesChanged, this, [this](const std::vector<CaptureMode>& modes) {
        capture_modes_ = modes;
        emit captureModesChanged();
    });

    connect(view_model_.get(), &ProfilingViewModel::CurrentCaptureModeChanged, this, [this](const uint32_t mode) {
        current_capture_mode_ = mode;
        emit captureModesChanged();
        emit renderOpCountChanged();
    });

    // -- Capture progress --------------------------------------------------------------
    connect(view_model_.get(), &ProfilingViewModel::ShowProgressUi, this, [this] {
        if (!capturing_)
        {
            capturing_ = true;
            emit capturingChanged();
        }
    });

    connect(view_model_.get(), &ProfilingViewModel::ShowCaptureUi, this, [this] {
        if (capturing_)
        {
            capturing_ = false;
            emit capturingChanged();
        }
    });

    connect(view_model_.get(), &ProfilingViewModel::ExecutableMissing, this, [](const QString&) {
        rdp::Notifications::Critical(QObject::tr("Radeon GPU Profiler missing"),
                                     QObject::tr("Radeon GPU Profiler executable not found. Please specify path in settings."));
    });

    // A profile that failed validation is worth nothing, so the user is offered its removal.
    connect(view_model_.get(), &ProfilingViewModel::InvalidTraceCounterData, this, [](const QString& path, const RgpFileValidatorStatus status) {
        QString message;
        switch (status)
        {
        case kRgpFileValidatorStatusSpmError:
            message = QObject::tr("Profile file does not contain valid SPM counter data. Delete the profile?");
            break;
        case kRgpFileValidatorStatusMissingChunkError:
            message = QObject::tr("Profile file is missing one or more required chunks. Delete the profile?");
            break;
        default:
            message = QObject::tr("Profile file is invalid. Delete the profile?");
            break;
        }

        rdp::Notifications::Question(QObject::tr("Invalid profile detected"), message, [path](const bool accepted) {
            if (accepted)
            {
                if (QFile file(path); file.exists())
                {
                    file.remove();
                }
            }
        });
    });

    // -- Hotkey and delay --------------------------------------------------------------
    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::CaptureShortcutChanged, this, [this](const GlobalShortcut& shortcut) {
        shortcut_ = shortcut;
        emit shortcutChanged();
    });

    connect(&GlobalShortcutManager::Instance(), &GlobalShortcutManager::ShortcutTriggered, this, [this](const GlobalShortcut& shortcut) {
        if (shortcut == shortcut_.id)
        {
            view_model_->RequestBeginTrace();
        }
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::ShouldDelayCaptureChanged, this, [this](const bool should_delay) {
        should_delay_capture_ = should_delay;
        emit delayChanged();
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::CaptureDelayChanged, this, [this](const uint32_t delay_ms) {
        capture_delay_ms_ = static_cast<int>(delay_ms);
        emit delayChanged();
    });

    // -- Settings ----------------------------------------------------------------------
    const auto on_settings_changed = [this] { emit settingsChanged(); };

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::InstructionTracingEnabledChanged, this, [this, on_settings_changed](const bool enabled) {
        instruction_tracing_enabled_ = enabled;
        on_settings_changed();
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::SpmCaptureEnabledChanged, this, [this, on_settings_changed](const bool enabled) {
        spm_capture_enabled_ = enabled;
        on_settings_changed();
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::SpmCaptureSupportedChanged, this, [this, on_settings_changed](const bool supported) {
        spm_capture_supported_ = supported;
        on_settings_changed();
    });

    connect(
        userdata_view_model_.get(), &ProfilingUserdataViewModel::ShaderInstrumentationEnabledChanged, this, [this, on_settings_changed](const bool enabled) {
            shader_instrumentation_enabled_ = enabled;
            on_settings_changed();
        });

    connect(userdata_view_model_.get(),
            &ProfilingUserdataViewModel::EditShaderInstrumentationEnabledChanged,
            this,
            [this, on_settings_changed](const bool editable) {
                shader_instrumentation_editable_ = editable;
                on_settings_changed();
            });

    connect(userdata_view_model_.get(),
            &ProfilingUserdataViewModel::SqttProfileIndexChanged,
            this,
            [this, on_settings_changed](const devtrace::SqttBufferSizeProfiles profile) {
                sqtt_profile_index_ = static_cast<int>(profile);
                on_settings_changed();
            });

    connect(
        userdata_view_model_.get(), &ProfilingUserdataViewModel::AutoCaptureModeChanged, this, [this, on_settings_changed](const devtrace::AutoCaptureMode mode) {
            auto_capture_mode_ = static_cast<int>(mode);
            on_settings_changed();
        });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::ComputeAutoCaptureTimeChanged, this, [this, on_settings_changed](const uint32_t time_ms) {
        compute_auto_capture_time_ms_ = static_cast<int>(time_ms);
        on_settings_changed();
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::DispatchStartChanged, this, [this, on_settings_changed](const uint32_t dispatch_start) {
        dispatch_start_index_ = static_cast<int>(dispatch_start);
        on_settings_changed();
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::DispatchCountChanged, this, [this, on_settings_changed](const uint32_t dispatch_count) {
        dispatch_count_ = static_cast<int>(dispatch_count);
        on_settings_changed();
        emit renderOpCountChanged();
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::DrawCountChanged, this, [this](const uint32_t draw_count) {
        draw_count_ = static_cast<int>(draw_count);
        emit renderOpCountChanged();
    });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::FrameCaptureIndexChanged, this, [this, on_settings_changed](const uint32_t frame_index) {
        frame_capture_index_ = static_cast<int>(frame_index);
        on_settings_changed();
    });

    connect(
        userdata_view_model_.get(), &ProfilingUserdataViewModel::PrelaunchSettingsEditableChanged, this, [this, on_settings_changed](const bool editable) {
            prelaunch_settings_editable_ = editable;
            on_settings_changed();
        });

    connect(
        userdata_view_model_.get(), &ProfilingUserdataViewModel::AutoCaptureSettingsEditableChanged, this, [this, on_settings_changed](const bool editable) {
            auto_capture_settings_editable_ = editable;
            on_settings_changed();
        });

    connect(userdata_view_model_.get(), &ProfilingUserdataViewModel::ShaderInstrumentationFailedToSet, this, [] {
        rdp::Notifications::Critical(
            QObject::tr("Unable to toggle shader instrumentation"),
            QObject::tr(
                "An unexpected error was encountered toggling shader instrumentation. Please relaunch Radeon Developer Panel to continue using this feature."));
    });

    // The profiling view model forwards the driver's feature support to the userdata model,
    // which is what decides whether SPM and shader instrumentation can be offered.
    connect(view_model_.get(),
            &ProfilingViewModel::TraceSupportChanged,
            userdata_view_model_.get(),
            &ProfilingUserdataViewModel::HandleTraceSupportChanged);

    // Pull the current state out of both models.
    userdata_view_model_->OnBind();
    view_model_->Update();

}

QAbstractItemModel* ProfilingCaptureViewModel::CaptureTargets() const
{
    return capture_targets_;
}

int ProfilingCaptureViewModel::CaptureTargetIndex() const
{
    const uint16_t target = view_model_->GetCaptureTarget();
    for (int i = 0; i < capture_targets_->rowCount({}); ++i)
    {
        if (capture_targets_->GetConnectionIdForIndex(i) == target)
        {
            return i;
        }
    }

    return -1;
}

void ProfilingCaptureViewModel::SetCaptureTargetIndex(const int index)
{
    if (index < 0 || index >= capture_targets_->rowCount({}))
    {
        return;
    }

    view_model_->SetCaptureTarget(capture_targets_->GetConnectionIdForIndex(index));
}

bool ProfilingCaptureViewModel::IsCaptureTargetSelectable() const
{
    return capture_targets_->IsSelectionEnabled();
}

QStringList ProfilingCaptureViewModel::CaptureModes() const
{
    QStringList names;
    names.reserve(static_cast<qsizetype>(capture_modes_.size()));
    for (const CaptureMode& mode : capture_modes_)
    {
        names.push_back(mode.text);
    }

    return names;
}

int ProfilingCaptureViewModel::CaptureModeIndex() const
{
    for (size_t i = 0; i < capture_modes_.size(); ++i)
    {
        if (capture_modes_[i].mode == current_capture_mode_)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void ProfilingCaptureViewModel::SetCaptureModeIndex(const int index)
{
    if (index < 0 || index >= static_cast<int>(capture_modes_.size()))
    {
        return;
    }

    const uint32_t mode = capture_modes_[static_cast<size_t>(index)].mode;
    if (mode == current_capture_mode_)
    {
        return;
    }

    view_model_->SetCaptureMode(mode);
    current_capture_mode_ = mode;

    emit captureModesChanged();
    emit renderOpCountChanged();
}

bool ProfilingCaptureViewModel::IsCaptureModeSelectable() const
{
    // A single mode is not a choice, and nothing can be chosen mid-capture.
    return IsCaptureEnabled() && capture_modes_.size() > 1;
}

bool ProfilingCaptureViewModel::IsRenderOpCountVisible() const
{
    const auto mode = static_cast<devtrace::RgpCaptureMode>(current_capture_mode_);
    return mode == devtrace::RgpCaptureMode::kDraw || mode == devtrace::RgpCaptureMode::kDispatch;
}

QString ProfilingCaptureViewModel::RenderOpCountLabel() const
{
    return static_cast<devtrace::RgpCaptureMode>(current_capture_mode_) == devtrace::RgpCaptureMode::kDraw ? QObject::tr("Draw count:")
                                                                                                          : QObject::tr("Dispatch count:");
}

int ProfilingCaptureViewModel::RenderOpCount() const
{
    return static_cast<devtrace::RgpCaptureMode>(current_capture_mode_) == devtrace::RgpCaptureMode::kDraw ? draw_count_ : dispatch_count_;
}

void ProfilingCaptureViewModel::SetRenderOpCount(const int count)
{
    if (count == RenderOpCount())
    {
        return;
    }

    // The one spin box stands for whichever count the current mode uses.
    if (static_cast<devtrace::RgpCaptureMode>(current_capture_mode_) == devtrace::RgpCaptureMode::kDraw)
    {
        userdata_view_model_->HandleDrawCountChanged(static_cast<uint32_t>(count));
    }
    else if (static_cast<devtrace::RgpCaptureMode>(current_capture_mode_) == devtrace::RgpCaptureMode::kDispatch)
    {
        userdata_view_model_->HandleDispatchCountChanged(static_cast<uint32_t>(count));
    }
}

bool ProfilingCaptureViewModel::IsCapturing() const
{
    return capturing_;
}

QString ProfilingCaptureViewModel::CaptureShortcut() const
{
    return QKeySequence(shortcut_.sequence).toString(QKeySequence::NativeText);
}

bool ProfilingCaptureViewModel::IsListeningForShortcut() const
{
    return shortcut_window_ != nullptr;
}

bool ProfilingCaptureViewModel::IsShortcutSupported()
{
#ifdef Q_OS_LINUX
    // Global hotkey capture needs an X11 dependency the panel does not take.
    return false;
#else
    return true;
#endif
}

bool ProfilingCaptureViewModel::IsDelayCaptureEnabled() const
{
    return should_delay_capture_;
}

void ProfilingCaptureViewModel::SetDelayCaptureEnabled(const bool enabled)
{
    if (enabled == should_delay_capture_)
    {
        return;
    }

    userdata_view_model_->HandleShouldDelayCaptureChanged(enabled);
}

int ProfilingCaptureViewModel::CaptureDelayMs() const
{
    return capture_delay_ms_;
}

void ProfilingCaptureViewModel::SetCaptureDelayMs(const int delay_ms)
{
    if (delay_ms == capture_delay_ms_)
    {
        return;
    }

    userdata_view_model_->HandleCaptureDelayChanged(static_cast<uint32_t>(delay_ms));
}

bool ProfilingCaptureViewModel::IsInstructionTracingEnabled() const
{
    return instruction_tracing_enabled_;
}

void ProfilingCaptureViewModel::SetInstructionTracingEnabled(const bool enabled)
{
    if (enabled != instruction_tracing_enabled_)
    {
        userdata_view_model_->HandleInstructionTracingEnabledChanged(enabled);
    }
}

bool ProfilingCaptureViewModel::IsSpmCaptureEnabled() const
{
    return spm_capture_enabled_;
}

void ProfilingCaptureViewModel::SetSpmCaptureEnabled(const bool enabled)
{
    if (enabled != spm_capture_enabled_)
    {
        userdata_view_model_->HandleSpmCaptureEnabledChanged(enabled);
    }
}

bool ProfilingCaptureViewModel::IsSpmCaptureSupported() const
{
    return spm_capture_supported_;
}

bool ProfilingCaptureViewModel::IsShaderInstrumentationEnabled() const
{
    return shader_instrumentation_enabled_;
}

void ProfilingCaptureViewModel::SetShaderInstrumentationEnabled(const bool enabled)
{
    if (enabled != shader_instrumentation_enabled_)
    {
        userdata_view_model_->HandleShaderInstrumentationEnabledChanged(enabled);
    }
}

bool ProfilingCaptureViewModel::IsShaderInstrumentationEditable() const
{
    return shader_instrumentation_editable_;
}

QStringList ProfilingCaptureViewModel::SqttProfiles()
{
    return kSqttProfileNames;
}

int ProfilingCaptureViewModel::SqttProfileIndex() const
{
    return sqtt_profile_index_;
}

void ProfilingCaptureViewModel::SetSqttProfileIndex(const int index)
{
    if (index != sqtt_profile_index_ && index >= 0 && index < kSqttProfileNames.size())
    {
        userdata_view_model_->HandleSqttProfileIndexChanged(static_cast<devtrace::SqttBufferSizeProfiles>(index));
    }
}

QStringList ProfilingCaptureViewModel::AutoCaptureModes()
{
    return kAutoCaptureModeNames;
}

int ProfilingCaptureViewModel::AutoCaptureMode() const
{
    return auto_capture_mode_;
}

void ProfilingCaptureViewModel::SetAutoCaptureMode(const int mode)
{
    if (mode != auto_capture_mode_ && mode >= 0 && mode < kAutoCaptureModeNames.size())
    {
        userdata_view_model_->HandleAutoCaptureModeChanged(static_cast<devtrace::AutoCaptureMode>(mode));
    }
}

int ProfilingCaptureViewModel::ComputeAutoCaptureTimeMs() const
{
    return compute_auto_capture_time_ms_;
}

void ProfilingCaptureViewModel::SetComputeAutoCaptureTimeMs(const int time_ms)
{
    if (time_ms != compute_auto_capture_time_ms_)
    {
        userdata_view_model_->HandleComputeAutoCaptureTimeChanged(static_cast<uint32_t>(time_ms));
    }
}

int ProfilingCaptureViewModel::DispatchStartIndex() const
{
    return dispatch_start_index_;
}

void ProfilingCaptureViewModel::SetDispatchStartIndex(const int dispatch_start)
{
    if (dispatch_start != dispatch_start_index_)
    {
        userdata_view_model_->HandleDispatchStartChanged(static_cast<uint32_t>(dispatch_start));
    }
}

int ProfilingCaptureViewModel::DispatchCount() const
{
    return dispatch_count_;
}

void ProfilingCaptureViewModel::SetDispatchCount(const int dispatch_count)
{
    if (dispatch_count != dispatch_count_)
    {
        userdata_view_model_->HandleDispatchCountChanged(static_cast<uint32_t>(dispatch_count));
    }
}

int ProfilingCaptureViewModel::FrameCaptureIndex() const
{
    return frame_capture_index_;
}

void ProfilingCaptureViewModel::SetFrameCaptureIndex(const int frame_index)
{
    if (frame_index != frame_capture_index_)
    {
        userdata_view_model_->HandleFrameCaptureIndexChanged(static_cast<uint32_t>(frame_index));
    }
}

int ProfilingCaptureViewModel::FrameIndexMinimum()
{
    return kFrameIndexMinimum;
}

int ProfilingCaptureViewModel::DispatchIndexMinimum()
{
    return kDispatchIndexMinimum;
}

bool ProfilingCaptureViewModel::ArePrelaunchSettingsEditable() const
{
    return prelaunch_settings_editable_;
}

bool ProfilingCaptureViewModel::AreAutoCaptureSettingsEditable() const
{
    return auto_capture_settings_editable_;
}

void ProfilingCaptureViewModel::SetDefaultCaptureMode()
{
    view_model_->UpdateDefaultCaptureMode();
}

void ProfilingCaptureViewModel::BeginShortcutCapture(QObject* window)
{
    if (window == nullptr || window == shortcut_window_)
    {
        return;
    }

    EndShortcutCapture();

    shortcut_window_ = window;
    shortcut_window_->installEventFilter(this);

    emit shortcutChanged();
}

void ProfilingCaptureViewModel::EndShortcutCapture()
{
    if (shortcut_window_ == nullptr)
    {
        return;
    }

    shortcut_window_->removeEventFilter(this);
    shortcut_window_ = nullptr;

    emit shortcutChanged();
}

void ProfilingCaptureViewModel::ResetShortcut()
{
    ApplyShortcut(ProfilingUserdataViewModel::GetDefaultShortcut());
}

bool ProfilingCaptureViewModel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != shortcut_window_ || event->type() != QEvent::KeyPress)
    {
        return QObject::eventFilter(watched, event);
    }

    const auto* key_event = static_cast<QKeyEvent*>(event);
    if (kIgnoredShortcutKeys.contains(key_event->key()))
    {
        // A modifier on its own is not a hotkey; keep listening.
        return true;
    }

    GlobalShortcut shortcut = shortcut_;
    shortcut.sequence       = key_event->key();
    shortcut.native_key     = static_cast<int>(key_event->nativeVirtualKey());

    const Qt::KeyboardModifiers modifiers = key_event->modifiers();
    if (modifiers & Qt::ShiftModifier)
    {
        shortcut.sequence += Qt::SHIFT;
    }
    if (modifiers & Qt::ControlModifier)
    {
        shortcut.sequence += Qt::CTRL;
    }
    if (modifiers & Qt::AltModifier)
    {
        shortcut.sequence += Qt::ALT;
    }
    if (modifiers & Qt::MetaModifier)
    {
        shortcut.sequence += Qt::META;
    }

    EndShortcutCapture();
    ApplyShortcut(shortcut);

    return true;
}

void ProfilingCaptureViewModel::UpdatePrelaunchEditableState()
{
    // The connections map can still hold an entry for a process that has gone, so the
    // source's own status is what decides.
    userdata_view_model_->HandlePrelaunchSettingsEditableChanged(view_model_->GetSourceStatus().pid == 0);
}

void ProfilingCaptureViewModel::ApplyShortcut(const GlobalShortcut& shortcut)
{
    // The hotkey is global, so another application may already own it.
    if (!GlobalShortcutManager::Instance().Register(shortcut))
    {
        rdp::Notifications::Critical(QObject::tr("Invalid shortcut"),
                                     QObject::tr("Shortcut '%1' is already in use by another feature or application.")
                                         .arg(QKeySequence(shortcut.sequence).toString(QKeySequence::NativeText)));

        emit shortcutChanged();
        return;
    }

    shortcut_ = shortcut;
    userdata_view_model_->HandleCaptureShortcutChanged(shortcut);

    emit shortcutChanged();
}
