pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Theme

// The Device Clocks module's view. The front-end instantiates this and sets
// viewModel to the module's DeviceClocksViewModel.
Flickable {
    id: root

    // Set by the front-end; typed as var so this file does not have to import the
    // module's C++ types.
    property var viewModel

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    contentWidth: width
    contentHeight: clocks.implicitHeight

    ScrollBar.vertical: RdpScrollBar {}

    ColumnLayout {
        id: clocks

        width: root.width
        spacing: 8

        RdpLabel {
            Layout.leftMargin: Theme.contentMargin * 2
            Layout.topMargin: Theme.contentMargin * 2
            text: qsTr("Clock stability")
            font.bold: true
        }

        RdpLabel {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.contentMargin * 2
            Layout.rightMargin: Theme.contentMargin * 2
            text: qsTr("Due to desire to operate within reasonable power envelopes, modern GPUs employ techniques which alter the frequencies of their clocks dynamically. This can make performance optimization difficult.")
            wrapMode: Text.WordWrap
        }

        RdpLabel {
            Layout.leftMargin: Theme.contentMargin * 2
            text: qsTr("During Profiling captures, clocks will automatically be set to peak.")
            font.bold: true
            color: Theme.statusWarning
        }

        EmptyStateLabel {
            Layout.fillWidth: true
            Layout.topMargin: Theme.contentMargin * 3
            Layout.leftMargin: Theme.contentMargin * 2
            Layout.rightMargin: Theme.contentMargin * 2
            visible: text.length > 0
            text: root.viewModel ? root.viewModel.emptyMessage : ""
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.contentMargin * 2
            Layout.rightMargin: Theme.contentMargin * 2
            Layout.topMargin: Theme.contentMargin * 2
            Layout.bottomMargin: Theme.contentMargin * 2
            spacing: 24

            Repeater {
                model: root.viewModel ? root.viewModel.gpus : null

                ColumnLayout {
                    id: deviceColumn

                    required property int index
                    required property string name
                    required property var modes
                    required property int currentMode

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 8

                    RdpLabel {
                        Layout.fillWidth: true
                        text: deviceColumn.name
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // The driver only reports a mode it recognises; anything else leaves
                    // every option unselected and says so.
                    RdpLabel {
                        Layout.fillWidth: true
                        visible: deviceColumn.currentMode < 0
                        text: qsTr("This device is currently using an unknown clock mode.")
                        font.bold: true
                        color: Theme.statusWarning
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Repeater {
                        model: deviceColumn.modes

                        ClockModeCard {
                            id: modeCard

                            required property int index
                            required property var modelData

                            Layout.fillWidth: true
                            modeName: modeCard.modelData.name
                            description: modeCard.modelData.description
                            shaderClock: modeCard.modelData.shaderClock
                            memoryClock: modeCard.modelData.memoryClock
                            selected: deviceColumn.currentMode === modeCard.index

                            onModeSelected: root.viewModel.RequestMode(deviceColumn.index, modeCard.index)
                        }
                    }
                }
            }
        }
    }
}
