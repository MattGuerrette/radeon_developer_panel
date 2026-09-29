import QtQuick

import Rdp.Theme

// Sidebar / module pane with a clickable header and a collapsible body.
Rectangle {
    id: root

    property string title
    property bool expanded: true
    property int contentPadding: 10
    property alias headerButtons: headerButtonRow.children

    default property alias contentData: body.data

    implicitHeight: column.height
    color: Theme.paneBackground
    border.width: 1
    border.color: Theme.paneBorder

    Column {
        id: column

        width: root.width

        Item {
            id: header

            width: parent.width
            height: Theme.paneHeaderHeight

            TapHandler {
                onTapped: root.expanded = !root.expanded
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            RdpIcon {
                id: chevron

                anchors.left: parent.left
                anchors.leftMargin: root.contentPadding
                anchors.verticalCenter: parent.verticalCenter
                width: 11
                height: 11
                icon: root.expanded ? Icons.chevronDown : Icons.chevronRight
                color: Theme.windowText
            }

            Text {
                anchors.left: chevron.right
                anchors.leftMargin: 8
                anchors.right: headerButtonRow.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: root.title
                font.pointSize: Theme.bodyPointSize
                font.bold: true
                color: Theme.windowText
                elide: Text.ElideRight
                renderType: Text.NativeRendering
            }

            Row {
                id: headerButtonRow

                anchors.right: parent.right
                anchors.rightMargin: root.contentPadding
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6
            }
        }

        Item {
            width: parent.width
            height: root.expanded ? body.childrenRect.height + root.contentPadding * 2 : 0
            visible: height > 0

            Item {
                id: body

                x: root.contentPadding
                y: root.contentPadding
                width: parent.width - root.contentPadding * 2
                implicitHeight: childrenRect.height
            }
        }
    }
}
