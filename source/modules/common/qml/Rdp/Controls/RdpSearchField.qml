import QtQuick

import Rdp.Theme

RdpTextField {
    id: control

    leftPadding: 26

    RdpIcon {
        anchors.left: parent.left
        anchors.leftMargin: 7
        anchors.verticalCenter: parent.verticalCenter
        width: 12
        height: 12
        icon: Icons.magnifier
        color: Theme.placeholderText
    }
}
