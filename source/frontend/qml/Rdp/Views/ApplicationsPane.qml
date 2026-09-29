pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

CollapsiblePane {
    id: root

    property int selectedIndex: 0

    // Choosing an application is what gives every module its output folder, so the
    // selection is pushed down as soon as it changes and once on start-up.
    onSelectedIndexChanged: Backend.SetDisplayApplicationRow(root.selectedIndex)

    title: qsTr("Applications")

    headerButtons: [
        RdpIconButton {
            icon_: Icons.circlePlus
            onClicked: addApplicationDialog.open()
        },
        RdpIconButton {
            icon_: Icons.xmark
            enabled: root.selectedIndex >= 0 && Backend.applicationModel.rowCount() > 0
            onClicked: Backend.applicationModel.RemoveApplicationAtRow(root.selectedIndex)
        }
    ]

    AddApplicationDialog {
        id: addApplicationDialog

        onAccepted: Backend.AddApplication(applicationName)
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(72, applicationList.contentHeight + 2)
            color: Theme.base
            border.width: 1
            border.color: Theme.separator

            ListView {
                id: applicationList

                anchors.fill: parent
                anchors.margins: 1
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                model: Backend.applicationModel

                ScrollBar.vertical: RdpScrollBar {}

                delegate: Rectangle {
                    id: applicationDelegate

                    required property int index
                    required property string name
                    required property bool isAlive

                    width: ListView.view.width
                    height: 24
                    color: root.selectedIndex === index ? Theme.highlight : "transparent"

                    TapHandler {
                        onTapped: root.selectedIndex = applicationDelegate.index
                    }

                    StatusDot {
                        id: dot

                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        width: 8
                        height: 8
                        online: applicationDelegate.isAlive
                    }

                    Text {
                        anchors.left: dot.right
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: applicationDelegate.name
                        font.pointSize: Theme.bodyPointSize
                        color: root.selectedIndex === applicationDelegate.index
                               ? Theme.highlightedText : Theme.windowText
                        elide: Text.ElideRight
                        renderType: Text.NativeRendering
                    }
                }
            }

            EmptyStateLabel {
                anchors.centerIn: parent
                width: parent.width - 16
                visible: applicationList.count === 0
                text: qsTr("No applications added.")
            }
        }
    }
}
