pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

Item {
    id: root

    readonly property var history: Backend.connectionHistory

    // A blank host name means "connect locally", which is how the Qt Widgets
    // front-end decided between ConnectToolLocal() and ConnectToolRemote().
    readonly property bool remoteRequested: hostNameField.text.length > 0

    function loadSelectedEntry() {
        const entry = root.history.EntryAt(root.history.currentIndex);
        if (entry.isLocal === undefined)
            return;

        if (entry.isLocal) {
            hostNameField.text = "";
            nicknameField.text = "";
            portSpinBox.value = 27300;
        } else {
            hostNameField.text = entry.hostname;
            nicknameField.text = entry.nickname;
            portSpinBox.value = entry.port;
        }
    }

    Component.onCompleted: root.loadSelectedEntry()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.contentMargin
        spacing: 10

        RdpLabel {
            text: {
                if (Backend.connecting)
                    return qsTr("Connecting...");
                if (Backend.connected)
                    return Backend.connectionDescription.length > 0
                        ? qsTr("Connected to %1").arg(Backend.connectionDescription)
                        : qsTr("Connected");
                return qsTr("No connection");
            }
        }

        RdpLabel {
            Layout.fillWidth: true
            text: Backend.connected
                  ? qsTr("Connected to a Radeon Developer Service.")
                  : qsTr("Open a new connection to a Radeon Developer Service.")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6
            spacing: 8

            RdpLabel {
                text: qsTr("Connection:")
            }

            RdpComboBox {
                id: historyCombo

                Layout.fillWidth: true
                Layout.maximumWidth: 520
                model: root.history
                textRole: "description"
                currentIndex: root.history.currentIndex

                onActivated: {
                    root.history.currentIndex = currentIndex;
                    root.loadSelectedEntry();
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Row {
            Layout.fillWidth: true

            RdpTabButton {
                text: qsTr("New connection")
                checked: true
                checkable: false
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.base
            border.width: 1
            border.color: Theme.separator

            GridLayout {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 16
                columns: 2
                columnSpacing: 10
                rowSpacing: 8

                RdpLabel {
                    text: qsTr("Host Name")
                }

                RdpTextField {
                    id: hostNameField

                    Layout.preferredWidth: 460
                    placeholderText: qsTr("Leave empty to connect locally")
                }

                RdpLabel {
                    text: qsTr("Port Number")
                }

                RdpSpinBox {
                    id: portSpinBox

                    Layout.preferredWidth: 460
                    from: 1
                    to: 65535
                    value: 27300
                }

                RdpLabel {
                    text: qsTr("Nickname")
                }

                RdpTextField {
                    id: nicknameField

                    Layout.preferredWidth: 460
                    placeholderText: qsTr("Optional")
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            RdpCheckBox {
                text: qsTr("Connect on startup")
                checked: root.history.autoConnectEnabled
                onToggled: root.history.autoConnectEnabled = checked
            }

            RdpCheckBox {
                text: qsTr("Disable client timeout")
                // The router only honours this for a local connection, and only while
                // nothing is connected through it yet.
                enabled: Backend.canDisableClientTimeout
                checked: Backend.clientTimeoutDisabled
                onToggled: Backend.clientTimeoutDisabled = checked
            }

            RdpButton {
                text: qsTr("Edit DD timeouts")
                onClicked: timeoutDialog.open()
            }

            Item {
                Layout.fillWidth: true
            }

            RdpButton {
                Layout.preferredWidth: 110
                // Both buttons are disabled mid-transition: a connection attempt can take
                // several seconds and still fail, and there is nothing useful to do until
                // it settles either way.
                enabled: !Backend.connected && !Backend.connecting
                text: qsTr("Connect")
                onClicked: {
                    if (root.remoteRequested)
                        root.history.ConnectRemote(hostNameField.text, nicknameField.text, portSpinBox.value);
                    else
                        root.history.ConnectLocal();
                }
            }

            RdpButton {
                Layout.preferredWidth: 110
                enabled: Backend.connected && !Backend.connecting
                text: qsTr("Disconnect")
                onClicked: Backend.Disconnect()
            }
        }
    }

    TimeoutDialog {
        id: timeoutDialog

        anchors.centerIn: parent
    }
}
