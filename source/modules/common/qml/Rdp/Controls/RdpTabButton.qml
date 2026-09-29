import QtQuick
import QtQuick.Controls

import Rdp.Theme

TabButton {
    id: control

    property var icon_: undefined
    property bool closable: false

    signal closeRequested

    implicitWidth: label.implicitWidth + 32 + (closable ? 18 : 0)
    implicitHeight: Theme.tabHeight
    // Pin every side: styles set the more specific horizontalPadding /
    // verticalPadding (Fusion uses horizontalPadding: 4), which would otherwise
    // win over the `padding` shorthand and inset the close button.
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    background: Rectangle {
        color: control.checked ? Theme.base : Theme.midlight

        Rectangle {
            anchors.fill: parent
            anchors.bottomMargin: control.checked ? 0 : 1
            color: "transparent"
            border.width: 1
            border.color: Theme.separator
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 1
            anchors.rightMargin: 1
            height: 1
            visible: control.checked
            color: Theme.base
        }
    }

    contentItem: Item {
        RdpIconButton {
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            width: 9
            height: 9
            visible: control.closable
            icon_: Icons.xmark
            iconColor: Theme.windowText
            onClicked: control.closeRequested()
        }

        Text {
            id: label

            anchors.centerIn: parent
            anchors.horizontalCenterOffset: control.closable ? 9 : 0
            text: control.text
            font.pointSize: Theme.tabPointSize
            font.bold: true
            color: control.checked ? Theme.windowText : Theme.disabledText
            renderType: Text.NativeRendering
        }
    }
}
