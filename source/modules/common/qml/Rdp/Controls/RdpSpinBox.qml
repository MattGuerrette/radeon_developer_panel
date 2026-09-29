import QtQuick
import QtQuick.Controls

import Rdp.Theme

SpinBox {
    id: control

    implicitHeight: Theme.controlHeight
    editable: true
    font.pointSize: Theme.tablePointSize

    // RDP shows raw numbers; the default formatter groups thousands.
    textFromValue: value => String(value)

    contentItem: TextInput {
        text: control.displayText
        font: control.font
        color: control.enabled ? Theme.text : Theme.disabledText
        selectionColor: Theme.highlight
        selectedTextColor: Theme.highlightedText
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter
        leftPadding: 6
        readOnly: !control.editable
        validator: control.validator
        renderType: Text.NativeRendering
    }

    up.indicator: Item {
        x: control.width - width - 1
        y: 1
        implicitWidth: 16
        implicitHeight: control.height / 2 - 1

        RdpIcon {
            anchors.centerIn: parent
            width: 7
            height: 7
            rotation: 180
            icon: Icons.chevronDown
            color: control.up.hovered ? Theme.accent : Theme.windowText
        }
    }

    down.indicator: Item {
        x: control.width - width - 1
        y: control.height / 2
        implicitWidth: 16
        implicitHeight: control.height / 2 - 1

        RdpIcon {
            anchors.centerIn: parent
            width: 7
            height: 7
            icon: Icons.chevronDown
            color: control.down.hovered ? Theme.accent : Theme.windowText
        }
    }

    background: Rectangle {
        color: control.enabled ? Theme.controlBackground : Theme.window
        border.width: 1
        border.color: control.activeFocus ? Theme.accent
                                          : control.enabled ? Theme.controlBorder : Theme.mid
    }
}
