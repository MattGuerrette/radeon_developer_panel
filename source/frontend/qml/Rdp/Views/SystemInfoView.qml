pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

ColumnLayout {
    id: root

    spacing: 8

    FolderDialog {
        id: exportChooser

        title: qsTr("Choose a folder to export the system information to")

        onAccepted: Backend.ExportSystemInfo(selectedFolder)
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: Theme.alternateBase
        border.width: 1
        border.color: Theme.separator

        ListView {
            id: entries

            anchors.fill: parent
            anchors.margins: 1
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: Backend.systemInfoEntries

            ScrollBar.vertical: RdpScrollBar {}

            delegate: Item {
                id: entry

                required property string key
                required property string value
                required property int depth
                required property bool isSection

                width: ListView.view.width
                // A group heading gets the space above it that separates it from the
                // fields of the group before.
                height: entry.isSection ? 30 : 20

                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 16 + entry.depth * 14
                    anchors.rightMargin: 16
                    anchors.bottomMargin: 2
                    spacing: 0

                    RdpLabel {
                        Layout.preferredWidth: entry.isSection ? implicitWidth : 220
                        text: entry.key
                        font.bold: entry.isSection
                        elide: Text.ElideRight
                    }

                    RdpLabel {
                        Layout.fillWidth: true
                        visible: !entry.isSection
                        text: entry.value
                        elide: Text.ElideRight
                    }
                }
            }
        }

        EmptyStateLabel {
            anchors.centerIn: parent
            width: parent.width - 40
            visible: entries.count === 0
            text: Backend.connected
                  ? qsTr("The system information has not loaded.")
                  : qsTr("Connect to a Radeon Developer Service to see the system information.")
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Item {
            Layout.fillWidth: true
        }

        RdpButton {
            text: qsTr("Export")
            enabled: Backend.systemInfoValid
            onClicked: exportChooser.open()
        }
    }
}
