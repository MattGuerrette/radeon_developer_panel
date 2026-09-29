import QtQuick

import Rdp.Theme

Rectangle {
    property bool online: false

    implicitWidth: 10
    implicitHeight: 10
    radius: width / 2
    color: online ? Theme.statusOnline : Theme.statusOffline
    antialiasing: true
}
