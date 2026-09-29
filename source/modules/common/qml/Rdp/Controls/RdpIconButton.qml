import QtQuick
import QtQuick.Controls

import Rdp.Theme

AbstractButton {
    id: control

    property var icon_: undefined
    property color iconColor: Theme.iconAccent
    property real iconScale: 1.0

    implicitWidth: Theme.iconButtonSize
    implicitHeight: Theme.iconButtonSize
    hoverEnabled: true
    opacity: !enabled ? 0.4 : control.hovered ? 0.8 : 1.0

    background: null

    contentItem: RdpIcon {
        icon: control.icon_
        color: control.iconColor
        scale: control.iconScale
    }
}
