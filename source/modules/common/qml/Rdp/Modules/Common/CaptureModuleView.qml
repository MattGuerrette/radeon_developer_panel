pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Theme

// The frame every capture module's view is built in: a scrollable options column on the
// left, and the module's output path plus the traces it has produced on the right.
//
// A module supplies its own settings panes through the default property; everything else
// here is driven by the shared CaptureViewModel.
SplitView {
    id: root

    // Set by the front-end after the component is created, so it cannot be required.
    property var viewModel

    // The heading over the output path, e.g. "Profiling output path".
    property string moduleName

    // Cleared by a module whose capture needs more than a button -- a target and a mode,
    // say -- so that it can put the control in its own pane instead.
    property bool showCaptureControl: true

    // The module's own settings panes, placed under the capture controls.
    default property alias settingsPanes: settingsColumn.data

    handle: SplitViewHandle {
        orientation: Qt.Horizontal
    }

    FolderDialog {
        id: outputChooser

        title: qsTr("Choose a folder to write captures to")

        onAccepted: root.viewModel.outputPath = selectedFolder.toString().replace("file:///", "")
    }

    Flickable {
        id: optionsFlickable

        SplitView.preferredWidth: 340
        SplitView.minimumWidth: 260
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: options.implicitHeight

        ScrollBar.vertical: RdpScrollBar {}

        ColumnLayout {
            id: options

            width: optionsFlickable.width
            spacing: 12

            CaptureStatusHeader {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.contentMargin
                Layout.rightMargin: Theme.contentMargin
                Layout.topMargin: Theme.contentMargin
                viewModel: root.viewModel
            }

            CollapsiblePane {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.contentMargin
                Layout.rightMargin: Theme.contentMargin
                visible: root.showCaptureControl && root.viewModel && root.viewModel.capturable
                title: qsTr("Capture")

                ColumnLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 6

                    RdpButton {
                        Layout.fillWidth: true
                        enabled: root.viewModel && root.viewModel.captureEnabled
                        text: qsTr("Capture")
                        onClicked: root.viewModel.Capture()
                    }

                    // Only present while a capture is running; cancelling is offered only
                    // for the stages the module says can be abandoned.
                    RdpProgressBar {
                        Layout.fillWidth: true
                        visible: root.viewModel && root.viewModel.progressVisible
                        indeterminate: root.viewModel && root.viewModel.progressIndeterminate
                        from: 0
                        to: 1
                        value: root.viewModel ? root.viewModel.progress : 0
                    }

                    RdpLabel {
                        Layout.fillWidth: true
                        visible: root.viewModel && root.viewModel.progressVisible
                        text: root.viewModel ? root.viewModel.progressText : ""
                        font.pointSize: Theme.smallPointSize
                        elide: Text.ElideRight
                    }

                    RdpButton {
                        Layout.fillWidth: true
                        visible: root.viewModel && root.viewModel.progressVisible && root.viewModel.progressCancellable
                        text: qsTr("Cancel")
                        onClicked: root.viewModel.Abort()
                    }
                }
            }

            ColumnLayout {
                id: settingsColumn

                Layout.fillWidth: true
                Layout.leftMargin: Theme.contentMargin
                Layout.rightMargin: Theme.contentMargin
                Layout.bottomMargin: Theme.contentMargin
                spacing: 12
            }
        }
    }

    Item {
        SplitView.fillWidth: true
        SplitView.minimumWidth: 320

        // The panes draw the divider themselves, as the sidebar and bottom bar do; the
        // handle stays transparent and only supplies the grab area.
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: Theme.separator
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.contentMargin
            spacing: 8

            RdpLabel {
                text: qsTr("%1 output path").arg(root.moduleName)
                font.pointSize: 12
                font.bold: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                // Shows the folder captures actually land in, and swaps to the path as
                // written -- macros and all -- as soon as it is being edited. This is what
                // the Qt Widgets output path row did.
                RdpTextField {
                    id: outputPathField

                    Layout.fillWidth: true
                    text: root.viewModel ? root.viewModel.outputPath : ""

                    onActiveFocusChanged: {
                        if (!root.viewModel)
                            return;

                        text = activeFocus ? root.viewModel.rawOutputPath : root.viewModel.outputPath;
                    }

                    onEditingFinished: root.viewModel.outputPath = text

                    Connections {
                        target: root.viewModel
                        // viewModel is set after this component is built, so its signals
                        // cannot be resolved when the Connections is created.
                        ignoreUnknownSignals: true

                        function onOutputPathChanged() {
                            if (!outputPathField.activeFocus)
                                outputPathField.text = root.viewModel.outputPath;
                        }
                    }
                }

                RdpButton {
                    Layout.preferredWidth: 34
                    leftPadding: 4
                    rightPadding: 4
                    text: qsTr("...")
                    onClicked: outputChooser.open()
                }
            }

            RdpLabel {
                Layout.fillWidth: true
                visible: root.viewModel && root.viewModel.outputPathInvalid
                text: qsTr("Captures cannot be written to this path.")
                font.pointSize: Theme.smallPointSize
                color: Theme.statusError
            }

            RdpLabel {
                Layout.topMargin: 8
                text: qsTr("Recently collected files")
                font.pointSize: 12
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.base
                border.width: 1
                border.color: Theme.separator
                clip: true

                ListView {
                    id: files

                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    model: root.viewModel ? root.viewModel.files : null

                    ScrollBar.vertical: RdpScrollBar {}

                    delegate: Rectangle {
                        id: file

                        required property int index
                        required property string name
                        required property string size
                        required property string modified

                        width: ListView.view.width
                        height: 20
                        color: file.index % 2 === 0 ? Theme.base : Theme.alternateBase

                        // Opening a capture is what the row is for, so the whole row is
                        // the target rather than a separate button.
                        TapHandler {
                            onDoubleTapped: root.viewModel.OpenFile(file.index)
                        }

                        Row {
                            anchors.fill: parent

                            LogCell {
                                width: file.width - 240
                                text: file.name
                                color: Theme.windowText
                            }

                            LogCell {
                                width: 90
                                text: file.size
                                color: Theme.windowText
                            }

                            LogCell {
                                width: 150
                                text: file.modified
                                color: Theme.windowText
                            }
                        }
                    }
                }

                EmptyStateLabel {
                    anchors.centerIn: parent
                    width: parent.width - 40
                    visible: files.count === 0
                    text: qsTr("Nothing has been captured to this folder yet.")
                }
            }
        }
    }
}
