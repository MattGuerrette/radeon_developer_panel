import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Theme

// Adds an executable to the application list, either by typing its name or by
// picking it off disk. Only the file name is kept: RDP matches connecting
// processes by executable name, not by path.
Dialog {
    id: root

    property alias applicationName: nameField.text

    anchors.centerIn: Overlay.overlay
    width: 420
    modal: true
    title: qsTr("Add application")
    standardButtons: Dialog.Ok | Dialog.Cancel

    onOpened: {
        nameField.clear();
        nameField.forceActiveFocus();
    }

    FileDialog {
        id: executableChooser

        title: qsTr("Choose an application executable")

        onAccepted: {
            const path = selectedFile.toString();
            nameField.text = path.slice(path.lastIndexOf("/") + 1);
        }
    }

    contentItem: ColumnLayout {
        spacing: 10

        RdpLabel {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Enter the executable name of the application to connect to.")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            RdpTextField {
                id: nameField

                Layout.fillWidth: true
                placeholderText: qsTr("application.exe")
                onAccepted: root.accept()
            }

            RdpButton {
                Layout.preferredWidth: 34
                leftPadding: 4
                rightPadding: 4
                text: qsTr("...")
                onClicked: executableChooser.open()
            }
        }
    }
}
