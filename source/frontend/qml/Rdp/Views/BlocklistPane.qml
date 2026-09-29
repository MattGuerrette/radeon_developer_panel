pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

CollapsiblePane {
    id: root

    property int selectedIndex: -1

    // The blocklist is per-platform and the platform is only known once the system
    // information has come back, so the title fills itself in on connection.
    title: Backend.blocklistPlatformName.length > 0
           ? qsTr("Blocklist [%1]").arg(Backend.blocklistPlatformName)
           : qsTr("Blocklist")
    expanded: false

    headerButtons: [
        RdpIconButton {
            icon_: Icons.circlePlus
            onClicked: addBlockedDialog.open()
        },
        RdpIconButton {
            icon_: Icons.xmark
            enabled: root.selectedIndex >= 0
            onClicked: {
                Backend.blocklistModel.RemoveAtRow(root.selectedIndex);
                root.selectedIndex = -1;
            }
        },
        RdpIconButton {
            icon_: Icons.refreshCircle
            onClicked: Backend.RestoreBlocklistDefaults()
        }
    ]

    AddApplicationDialog {
        id: addBlockedDialog

        title: qsTr("Block application")

        onAccepted: Backend.AddBlockedApplication(applicationName)
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 110
            color: Theme.base
            border.width: 1
            border.color: Theme.separator

            ListView {
                id: blocklist

                anchors.fill: parent
                anchors.margins: 1
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                model: Backend.blocklistModel

                ScrollBar.vertical: RdpScrollBar {}

                delegate: Rectangle {
                    id: blocklistDelegate

                    required property int index
                    required property string name

                    width: ListView.view.width
                    height: 22
                    color: root.selectedIndex === index ? Theme.highlight : "transparent"

                    TapHandler {
                        onTapped: root.selectedIndex = blocklistDelegate.index
                    }

                    Text {
                        anchors.fill: parent
                        leftPadding: 8
                        text: blocklistDelegate.name
                        font.pointSize: Theme.bodyPointSize
                        color: root.selectedIndex === blocklistDelegate.index
                               ? Theme.highlightedText : Theme.windowText
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        renderType: Text.NativeRendering
                    }
                }
            }

            EmptyStateLabel {
                anchors.centerIn: parent
                width: parent.width - 16
                visible: blocklist.count === 0
                text: qsTr("The blocklist is empty.")
            }
        }
    }
}
