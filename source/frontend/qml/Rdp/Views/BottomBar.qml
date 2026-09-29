pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Theme

Item {
    id: root

    property bool collapsed: false
    property int activeTab: 0

    readonly property int collapsedHeight: Theme.tabHeight

    implicitHeight: collapsedHeight

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.tabHeight

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                visible: !root.collapsed
                color: Theme.separator
            }

            Row {
                anchors.left: parent.left
                anchors.bottom: parent.bottom

                Repeater {
                    model: [qsTr("System information"), qsTr("Output log")]

                    RdpTabButton {
                        id: tabButton

                        required property int index
                        required property string modelData

                        text: modelData
                        checkable: false
                        checked: !root.collapsed && root.activeTab === index

                        onClicked: {
                            root.activeTab = tabButton.index;
                            root.collapsed = false;
                        }
                    }
                }
            }

            RdpIconButton {
                anchors.right: parent.right
                anchors.rightMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                width: 12
                height: 12
                visible: !root.collapsed
                icon_: Icons.minus
                iconColor: Theme.windowText
                onClicked: root.collapsed = true
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Theme.contentMargin
            visible: !root.collapsed
            currentIndex: root.activeTab

            SystemInfoView {}

            OutputLogView {}
        }
    }
}
