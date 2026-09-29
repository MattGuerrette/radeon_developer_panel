pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

CollapsiblePane {
    id: root

    title: qsTr("Available features")

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 2

        Repeater {
            model: Backend.captureModuleModel

            RowLayout {
                id: featureRow

                required property int index
                required property string displayName
                required property bool moduleEnabled
                required property bool incompatible
                required property string incompatibilityReason

                Layout.fillWidth: true
                Layout.preferredHeight: 26

                RdpLabel {
                    Layout.fillWidth: true
                    text: featureRow.displayName
                    color: featureRow.incompatible ? Theme.disabledText : Theme.windowText
                    elide: Text.ElideRight
                }

                RdpIcon {
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    visible: featureRow.incompatible
                    icon: Icons.warning
                    color: Theme.statusWarning

                    ToolTip.visible: incompatibleHover.hovered
                    ToolTip.text: featureRow.incompatibilityReason

                    HoverHandler {
                        id: incompatibleHover
                    }
                }

                // A module already in a tab is removed by closing that tab, so the pane
                // only ever offers the "add" direction.
                RdpIconButton {
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    visible: !featureRow.incompatible && !featureRow.moduleEnabled
                    icon_: Icons.circlePlus
                    onClicked: Backend.SetCaptureModuleEnabled(featureRow.index, true)
                }

                RdpIcon {
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    visible: featureRow.moduleEnabled
                    icon: Icons.checkmark
                    color: Theme.statusGood
                }
            }
        }

        EmptyStateLabel {
            Layout.fillWidth: true
            visible: Backend.captureModuleModel.rowCount() === 0
            text: qsTr("No features are available.")
        }
    }
}
