import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Theme

Rectangle {
    id: root

    implicitWidth: 300
    color: Theme.sidebar

    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: Theme.mid
    }

    Flickable {
        id: flickable

        anchors.fill: parent
        anchors.rightMargin: 1
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: panes.implicitHeight

        ScrollBar.vertical: RdpScrollBar {}

        ColumnLayout {
            id: panes

            width: flickable.width
            spacing: Theme.paneSpacing

            AvailableFeaturesPane {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.sidebarMargin
                Layout.rightMargin: Theme.sidebarMargin
                Layout.topMargin: Theme.sidebarMargin

            }

            ApplicationFiltersPane {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.sidebarMargin
                Layout.rightMargin: Theme.sidebarMargin
            }

            ApplicationsPane {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.sidebarMargin
                Layout.rightMargin: Theme.sidebarMargin
            }

            BlocklistPane {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.sidebarMargin
                Layout.rightMargin: Theme.sidebarMargin
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: Theme.sidebarMargin
            }
        }
    }
}
