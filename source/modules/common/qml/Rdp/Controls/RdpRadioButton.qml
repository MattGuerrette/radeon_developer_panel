import QtQuick
import QtQuick.Controls

import Rdp.Theme

RadioButton {
    id: control

    spacing: 6
    font.pointSize: Theme.bodyPointSize

    indicator: Rectangle {
        implicitWidth: 14
        implicitHeight: 14
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: width / 2
        antialiasing: true
        color: control.enabled ? Theme.controlBackground : Theme.window
        border.width: 1
        border.color: control.enabled ? Theme.controlBorder : Theme.mid

        Rectangle {
            anchors.centerIn: parent
            width: 7
            height: 7
            radius: width / 2
            antialiasing: true
            visible: control.checked
            color: control.enabled ? Theme.windowText : Theme.disabledText
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? Theme.windowText : Theme.disabledText
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
        renderType: Text.NativeRendering
    }
}
