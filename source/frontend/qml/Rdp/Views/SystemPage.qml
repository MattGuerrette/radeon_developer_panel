pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

// The SYSTEM view hosts the modules that work system-wide rather than per
// application connection. Unlike the CAPTURE tabs these are always present:
// a system module cannot be disabled, so there is nothing to close.
ColumnLayout {
    id: root

    property int currentTab: 0

    spacing: 0

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.tabHeight
        visible: systemTabs.count > 0

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Theme.separator
        }

        Row {
            anchors.left: parent.left
            anchors.bottom: parent.bottom

            Repeater {
                id: systemTabs

                model: Backend.systemModuleModel

                RdpTabButton {
                    id: systemTab

                    required property int index
                    required property string displayName

                    text: systemTab.displayName
                    checkable: false
                    checked: root.currentTab === systemTab.index
                    onClicked: root.currentTab = systemTab.index
                }
            }
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        EmptyStateLabel {
            anchors.centerIn: parent
            width: Math.min(parent.width - 40, 520)
            visible: systemTabs.count === 0
            text: qsTr("No system features are available.")
        }

        Repeater {
            model: Backend.systemModuleModel

            ModuleHost {
                id: systemModuleHost

                required property int index
                required property url qmlViewUrl
                required property var viewModel

                anchors.fill: parent
                visible: root.currentTab === systemModuleHost.index
                viewUrl: systemModuleHost.qmlViewUrl
                moduleViewModel: systemModuleHost.viewModel
            }
        }
    }
}
