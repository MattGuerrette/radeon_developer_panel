import QtQuick
import QtQuick.Controls

import Rdp.Theme

ScrollBar {
    id: control

    padding: 5
    visible: policy !== ScrollBar.AlwaysOff && size < 1.0

    contentItem: Rectangle {
        implicitWidth: 8
        implicitHeight: 8
        color: Theme.dark
    }

    background: null
}
