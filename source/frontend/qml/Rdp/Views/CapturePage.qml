pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

SplitView {
    id: root

    // The tabs are the enabled capture modules: enabling a feature in the sidebar opens
    // its tab, and closing a tab disables the feature. There is no separate "open tabs"
    // list to keep in step with the module model.
    readonly property var openModules: Backend.openCaptureModuleModel

    property int currentModule: 0

    orientation: Qt.Horizontal

    handle: SplitViewHandle {
        orientation: Qt.Horizontal
    }

    SidebarView {
        SplitView.preferredWidth: 300
        SplitView.minimumWidth: 220
        SplitView.maximumWidth: 480
    }

    SplitView {
        SplitView.fillWidth: true
        orientation: Qt.Vertical

        handle: SplitViewHandle {
            orientation: Qt.Vertical
        }

        Item {
            SplitView.fillHeight: true
            SplitView.minimumHeight: 200

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.tabHeight
                    visible: moduleTabs.count > 0

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
                            id: moduleTabs

                            model: root.openModules

                            RdpTabButton {
                                id: moduleTab

                                required property int index
                                required property string displayName

                                text: moduleTab.displayName
                                checkable: false
                                closable: true
                                checked: root.currentModule === index

                                onClicked: root.currentModule = moduleTab.index
                                onCloseRequested: Backend.CloseCaptureModule(moduleTab.index)
                            }
                        }
                    }

                    RdpIconButton {
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        width: 14
                        height: 14
                        icon_: Icons.bars
                        iconColor: Theme.windowText
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    EmptyStateLabel {
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 40, 520)
                        visible: moduleTabs.count === 0
                        text: qsTr("No features are enabled. Features can be enabled by loading a preset or by pressing the enable button in the available features pane.")
                    }

                    Repeater {
                        model: root.openModules

                        ModuleHost {
                            id: moduleHost

                            required property int index
                            required property url qmlViewUrl
                            required property var viewModel

                            anchors.fill: parent
                            visible: root.currentModule === moduleHost.index
                            viewUrl: moduleHost.qmlViewUrl
                            moduleViewModel: moduleHost.viewModel
                        }
                    }
                }
            }
        }

        BottomBar {
            id: bottomBar

            SplitView.preferredHeight: 260
            SplitView.minimumHeight: collapsed ? collapsedHeight : 160
            SplitView.maximumHeight: collapsed ? collapsedHeight : Number.POSITIVE_INFINITY
        }
    }

    // A closed tab must not leave the selection past the end of the list.
    Connections {
        target: root.openModules

        function onRowsRemoved() {
            root.currentModule = Math.max(0, Math.min(root.currentModule, moduleTabs.count - 1));
        }

        function onRowsInserted(parent, first) {
            // Enabling a feature is a deliberate act, so its tab becomes the current one.
            root.currentModule = first;
        }
    }
}
