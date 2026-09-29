import QtQuick
import QtQuick.Controls

import Rdp.Theme

// The red-outlined gear / bug / help buttons that sit on the navigation bar.
AbstractButton {
    id: control

    property var icon_: undefined

    // Held on while the page the button opens is the one being shown, which is how the
    // Qt Widgets front-end marked the gear while the settings page was open.
    property bool selected: false

    implicitWidth: 32
    implicitHeight: 28
    hoverEnabled: true

    background: Rectangle {
        color: "transparent"
        radius: 3
        border.width: 2
        border.color: control.hovered || control.selected ? Theme.accent : Theme.accentDim
    }

    contentItem: RdpIcon {
        icon: control.icon_
        color: control.hovered || control.selected ? Theme.navigationTextSelected : Theme.navigationTextHover
    }
}
