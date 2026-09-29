pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

CollapsiblePane {
    id: root

    title: qsTr("Application filters")

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 6

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 8
            rowSpacing: 6

            RdpLabel {
                text: qsTr("Auto connect:")
            }

            RdpComboBox {
                Layout.fillWidth: true
                model: Backend.autoConnectModel
                textRole: "name"
                currentIndex: Backend.autoConnectMode
                onActivated: Backend.autoConnectMode = currentIndex
            }

            RdpLabel {
                text: qsTr("API:")
            }

            RdpComboBox {
                Layout.fillWidth: true
                model: Backend.apiFilterModel
                textRole: "name"
                currentIndex: Backend.apiFilterIndex
                onActivated: Backend.apiFilterIndex = currentIndex

                // An API no enabled feature can capture stays visible but unselectable,
                // the way the Qt Widgets front-end greyed the row out.
                delegate: ItemDelegate {
                    id: apiDelegate

                    required property int index
                    required property string name
                    required property bool apiEnabled
                    required property string tooltip

                    width: ListView.view.width
                    text: apiDelegate.name
                    enabled: apiDelegate.apiEnabled
                    highlighted: apiDelegate.index === Backend.apiFilterIndex

                    ToolTip.visible: apiDelegateHover.hovered && apiDelegate.tooltip.length > 0
                    ToolTip.text: apiDelegate.tooltip

                    HoverHandler {
                        id: apiDelegateHover
                    }
                }
            }
        }

        RdpLabel {
            Layout.fillWidth: true
            Layout.topMargin: 4
            visible: text.length > 0
            text: Backend.autoConnectSummary
            font.pointSize: Theme.smallPointSize
            wrapMode: Text.WordWrap
        }
    }
}
