import QtQuick
import QtQuick.Controls

import Rdp.Theme

AbstractButton {
    id: control

    property bool selected: false

    implicitWidth: label.implicitWidth + 32
    implicitHeight: Theme.navigationBarHeight
    hoverEnabled: true

    background: Rectangle {
        color: "transparent"

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 4
            visible: control.selected || control.hovered
            color: control.selected ? Theme.accent : Theme.accentDim
        }
    }

    contentItem: Text {
        id: label

        text: control.text
        font.pointSize: Theme.navigationPointSize
        font.bold: true
        color: control.selected ? Theme.navigationTextSelected
                                : control.hovered ? Theme.navigationTextHover
                                                  : Theme.navigationText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        renderType: Text.NativeRendering
    }
}
