import QtQuick
import QtQuick.Controls

import Rdp.Theme

TextField {
    id: control

    implicitHeight: Theme.controlHeight
    // Pinned so vertical centring does not move with the active style's
    // padding (Basic uses 6, Fusion 4).
    leftPadding: 6
    rightPadding: 6
    topPadding: 6
    bottomPadding: 6
    font.pointSize: Theme.bodyPointSize
    color: enabled ? Theme.text : Theme.disabledText
    placeholderTextColor: Theme.placeholderText
    selectionColor: Theme.highlight
    selectedTextColor: Theme.highlightedText
    renderType: Text.NativeRendering

    background: Rectangle {
        color: control.enabled ? Theme.controlBackground : Theme.window
        border.width: 1
        border.color: control.activeFocus ? Theme.accent
                                          : control.enabled ? Theme.controlBorder : Theme.mid
    }
}
