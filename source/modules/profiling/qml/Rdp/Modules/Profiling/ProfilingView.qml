pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Modules.Common
import Rdp.Theme

// The Profiling module's view. The front-end instantiates this and sets viewModel to the
// module's ProfilingCaptureViewModel.
//
// The status, output path and captured-file list come from CaptureModuleView, which every
// capture module shares. The panes below are Profiling's own: what to capture, when to
// capture it, and how much detail to collect.
CaptureModuleView {
    id: root

    moduleName: qsTr("Profiling")

    // Capture is driven from here rather than from CaptureModuleView's generic control,
    // because a profile needs a target and a mode picked first.
    showCaptureControl: false

    CollapsiblePane {
        Layout.fillWidth: true
        title: qsTr("Capture")

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 6

            RdpLabel {
                text: qsTr("Capture target:")
            }

            RdpComboBox {
                Layout.fillWidth: true
                enabled: root.viewModel && root.viewModel.captureTargetSelectable
                model: root.viewModel ? root.viewModel.captureTargets : null
                textRole: "name"
                currentIndex: root.viewModel ? root.viewModel.captureTargetIndex : -1
                onActivated: root.viewModel.captureTargetIndex = currentIndex
            }

            RdpLabel {
                Layout.topMargin: 4
                text: qsTr("Capture mode:")
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                RdpComboBox {
                    Layout.fillWidth: true
                    enabled: root.viewModel && root.viewModel.captureModeSelectable
                    model: root.viewModel ? root.viewModel.captureModes : []
                    currentIndex: root.viewModel ? root.viewModel.captureModeIndex : -1
                    onActivated: root.viewModel.captureModeIndex = currentIndex
                }

                RdpButton {
                    enabled: root.viewModel && root.viewModel.captureEnabled
                    text: qsTr("Set default")
                    onClicked: root.viewModel.SetDefaultCaptureMode()

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("This will update the default capture mode for the connected application. Default capture modes are stored in presets and per-API.")
                }
            }

            // Only a draw or dispatch capture takes a count.
            RowLayout {
                Layout.fillWidth: true
                visible: root.viewModel && root.viewModel.renderOpCountVisible
                spacing: 6

                RdpLabel {
                    text: root.viewModel ? root.viewModel.renderOpCountLabel : ""
                }

                RdpSpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 2147483647
                    value: root.viewModel ? root.viewModel.renderOpCount : 1
                    onValueModified: root.viewModel.renderOpCount = value
                }
            }

            // While a capture runs, the button gives way to its progress.
            RdpButton {
                Layout.fillWidth: true
                Layout.topMargin: 4
                visible: !(root.viewModel && root.viewModel.capturing)
                enabled: root.viewModel && root.viewModel.captureEnabled
                text: qsTr("Capture profile")
                onClicked: root.viewModel.Capture()
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                visible: root.viewModel && root.viewModel.capturing
                spacing: 6

                RdpLabel {
                    Layout.fillWidth: true
                    text: root.viewModel ? root.viewModel.progressText : ""
                    elide: Text.ElideRight
                }

                RdpProgressBar {
                    Layout.fillWidth: true
                    // The trace source cannot know how long some stages take, and reports
                    // progress 0 for them.
                    indeterminate: root.viewModel && root.viewModel.progressIndeterminate
                    from: 0
                    to: 1
                    value: root.viewModel ? root.viewModel.progress : 0
                }

                RdpButton {
                    Layout.fillWidth: true
                    enabled: root.viewModel && root.viewModel.progressCancellable
                    text: qsTr("Cancel")
                    onClicked: root.viewModel.Abort()
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                visible: root.viewModel && root.viewModel.shortcutSupported
                spacing: 6

                RdpLabel {
                    text: qsTr("Hotkey")
                }

                ShortcutField {
                    Layout.fillWidth: true
                    viewModel: root.viewModel
                }
            }

            RdpCheckBox {
                Layout.fillWidth: true
                text: qsTr("Delay capture")
                enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                checked: root.viewModel && root.viewModel.delayCaptureEnabled
                onToggled: root.viewModel.delayCaptureEnabled = checked
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.viewModel && root.viewModel.delayCaptureEnabled
                spacing: 6

                RdpLabel {
                    text: qsTr("Delay (ms)")
                }

                RdpSpinBox {
                    Layout.fillWidth: true
                    enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                    from: 0
                    to: 2147483647
                    value: root.viewModel ? root.viewModel.captureDelayMs : 0
                    onValueModified: root.viewModel.captureDelayMs = value
                }
            }

            RdpCheckBox {
                Layout.fillWidth: true
                text: qsTr("Enable instruction tracing (may affect performance)")
                enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                checked: root.viewModel && root.viewModel.instructionTracingEnabled
                onToggled: root.viewModel.instructionTracingEnabled = checked
            }

            RdpCheckBox {
                Layout.fillWidth: true
                // Hidden outright on hardware that cannot collect counters.
                visible: root.viewModel && root.viewModel.spmCaptureSupported
                enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                text: qsTr("Collect counters")
                checked: root.viewModel && root.viewModel.spmCaptureEnabled
                onToggled: root.viewModel.spmCaptureEnabled = checked
            }
        }
    }

    CollapsiblePane {
        Layout.fillWidth: true
        title: qsTr("Auto capture")
        expanded: false

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 6

            RdpLabel {
                text: qsTr("Auto capture mode")
            }

            RdpComboBox {
                Layout.fillWidth: true
                // The driver reads the mode when the application launches.
                enabled: root.viewModel && root.viewModel.prelaunchSettingsEditable
                model: root.viewModel ? root.viewModel.autoCaptureModes : []
                currentIndex: root.viewModel ? root.viewModel.autoCaptureMode : 0
                onActivated: root.viewModel.autoCaptureMode = currentIndex
            }

            // Each mode asks for the fields it needs and no others: a frame index for a
            // frame capture, a dispatch range for a range capture, and a count and a time
            // for a timed one. Each row hides as a whole so it takes no space when the
            // mode does not use it.
            RowLayout {
                Layout.fillWidth: true
                visible: root.viewModel && root.viewModel.autoCaptureMode === 1
                spacing: 8

                RdpLabel {
                    Layout.preferredWidth: 130
                    text: qsTr("Frame index:")
                }

                RdpSpinBox {
                    Layout.fillWidth: true
                    enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                    from: root.viewModel ? root.viewModel.frameIndexMinimum : 5
                    to: 2147483647
                    value: root.viewModel ? root.viewModel.frameCaptureIndex : 0
                    onValueModified: root.viewModel.frameCaptureIndex = value
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.viewModel && (root.viewModel.autoCaptureMode === 2 || root.viewModel.autoCaptureMode === 3)
                spacing: 8

                RdpLabel {
                    Layout.preferredWidth: 130
                    text: qsTr("Dispatch count:")
                }

                RdpSpinBox {
                    Layout.fillWidth: true
                    enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                    from: 0
                    to: 2147483647
                    value: root.viewModel ? root.viewModel.dispatchCount : 0
                    onValueModified: root.viewModel.dispatchCount = value
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.viewModel && root.viewModel.autoCaptureMode === 2
                spacing: 8

                RdpLabel {
                    Layout.preferredWidth: 130
                    text: qsTr("Dispatch start index:")
                }

                RdpSpinBox {
                    Layout.fillWidth: true
                    enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                    from: root.viewModel ? root.viewModel.dispatchIndexMinimum : 1
                    to: 2147483647
                    value: root.viewModel ? root.viewModel.dispatchStartIndex : 1
                    onValueModified: root.viewModel.dispatchStartIndex = value
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.viewModel && root.viewModel.autoCaptureMode === 3
                spacing: 8

                RdpLabel {
                    Layout.preferredWidth: 130
                    text: qsTr("Capture time (ms):")
                }

                RdpSpinBox {
                    Layout.fillWidth: true
                    enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                    from: 0
                    to: 2147483647
                    value: root.viewModel ? root.viewModel.computeAutoCaptureTimeMs : 0
                    onValueModified: root.viewModel.computeAutoCaptureTimeMs = value
                }
            }
        }
    }

    CollapsiblePane {
        Layout.fillWidth: true
        title: qsTr("Advanced settings")
        expanded: false

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 6

            RdpLabel {
                text: qsTr("SQTT buffer size:")
            }

            RdpComboBox {
                Layout.fillWidth: true
                enabled: root.viewModel && root.viewModel.autoCaptureSettingsEditable
                model: root.viewModel ? root.viewModel.sqttProfiles : []
                currentIndex: root.viewModel ? root.viewModel.sqttProfileIndex : 0
                onActivated: root.viewModel.sqttProfileIndex = currentIndex
            }

            RdpCheckBox {
                Layout.fillWidth: true
                Layout.topMargin: 4
                text: qsTr("Enable shader instrumentation")
                enabled: root.viewModel && root.viewModel.shaderInstrumentationEditable
                checked: root.viewModel && root.viewModel.shaderInstrumentationEnabled
                onToggled: root.viewModel.shaderInstrumentationEnabled = checked

                ToolTip.visible: hovered
                ToolTip.text: qsTr("When enabled, additional instrumentation will be applied to shaders to gather more detailed performance information.\nOn RDNA4 or newer hardware, this also enables execution mask tracking.\nThis may affect application performance.")
            }
        }
    }
}
