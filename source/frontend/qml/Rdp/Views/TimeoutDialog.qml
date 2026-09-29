import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

// The DevDriver timeout editor reached from the CONNECTION page. Saving restarts
// RDP: the timeouts are read when the tool is created, so nothing else picks them up.
Dialog {
    id: root

    width: 460
    modal: true
    title: qsTr("DevDriver timeouts")
    standardButtons: Dialog.Cancel

    onOpened: {
        retrySpinBox.value = Backend.retryTimeoutMs;
        communicationSpinBox.value = Backend.communicationTimeoutMs;
        connectionSpinBox.value = Backend.connectionTimeoutMs;
    }

    contentItem: ColumnLayout {
        spacing: 12

        RdpLabel {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("These are the internal timeouts for DevDriver.\nA value of 0 will use the default timeout and a value of -1 will disable the timeout.\nWarning: These settings are for CSim use; adjust the timeouts in the driver builds similarly.")
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 8

            RdpLabel {
                text: qsTr("Retry timeout (ms)")
            }

            RdpSpinBox {
                id: retrySpinBox

                Layout.fillWidth: true
                from: -1
                to: 2147483647
            }

            RdpLabel {
                text: qsTr("Communication timeout (ms)")
            }

            RdpSpinBox {
                id: communicationSpinBox

                Layout.fillWidth: true
                from: -1
                to: 2147483647
            }

            RdpLabel {
                text: qsTr("Connection timeout (ms)")
            }

            RdpSpinBox {
                id: connectionSpinBox

                Layout.fillWidth: true
                from: -1
                to: 2147483647
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            RdpButton {
                text: qsTr("Save and restart")
                onClicked: {
                    Backend.SaveTimeouts(retrySpinBox.value, communicationSpinBox.value, connectionSpinBox.value);
                    root.close();
                }
            }

            RdpButton {
                text: qsTr("Reset and restart")
                onClicked: {
                    Backend.ResetTimeouts();
                    root.close();
                }
            }
        }
    }
}
