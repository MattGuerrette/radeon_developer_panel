import QtQuick

import Rdp.Theme

// The always-dark bar across the top of the main window.
Rectangle {
    id: root

    default property alias tabs: tabRow.children
    property alias trailing: trailingRow.children
    property alias online: statusDot.online

    implicitHeight: Theme.navigationBarHeight
    color: Theme.navigationBar

    StatusDot {
        id: statusDot

        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.verticalCenter: parent.verticalCenter
    }

    Row {
        id: tabRow

        anchors.left: statusDot.right
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }

    Row {
        id: trailingRow

        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10
    }
}
