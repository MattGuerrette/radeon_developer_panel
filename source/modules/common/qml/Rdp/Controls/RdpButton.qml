import QtQuick
import QtQuick.Controls

import Rdp.Theme

Button {
    id: control

    implicitHeight: Theme.controlHeight
    // Pinned so vertical centring does not move with the active style's
    // padding (Basic uses 6, Fusion 4).
    leftPadding: 12
    rightPadding: 12
    topPadding: 6
    bottomPadding: 6
    font.pointSize: Theme.bodyPointSize

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? Theme.windowText : Theme.disabledText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        renderType: Text.NativeRendering
    }

    background: Rectangle {
        color: !control.enabled ? Theme.window
                                : control.down ? Theme.midlight
                                               : control.hovered ? Theme.hover
                                                                 : Theme.controlBackground
        border.color: control.enabled ? Theme.controlBorder : Theme.mid
        border.width: 1
    }
}
