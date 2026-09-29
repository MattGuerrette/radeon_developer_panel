import QtQuick
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Theme

Rectangle {
    id: root

    property string modeName
    property string description
    property string shaderClock
    property string memoryClock
    property bool selected: false

    signal modeSelected

    implicitHeight: layout.implicitHeight + 24
    color: Theme.paneBackground
    border.width: 1
    border.color: Theme.paneBorder

    ColumnLayout {
        id: layout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        spacing: 12

        RdpRadioButton {
            text: root.modeName
            font.bold: true
            checked: root.selected
            onToggled: root.modeSelected()
        }

        RdpLabel {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            text: root.description
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20

            RdpLabel {
                text: qsTr("Shader clock: ")
                font.bold: true
            }

            RdpLabel {
                text: root.shaderClock
            }

            Item {
                Layout.fillWidth: true
            }

            RdpLabel {
                text: qsTr("Memory clock: ")
                font.bold: true
            }

            RdpLabel {
                text: root.memoryClock
            }
        }
    }
}
