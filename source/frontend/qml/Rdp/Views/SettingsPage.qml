pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

Flickable {
    id: root

    readonly property var settings: Backend.settings

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    contentWidth: width
    contentHeight: settingsColumn.implicitHeight + Theme.contentMargin * 2

    ScrollBar.vertical: RdpScrollBar {}

    // One chooser serves every row: the row that opened it is remembered in
    // targetRow, and the backend-test rows want a file while the settings-file
    // row wants a directory.
    FileDialog {
        id: fileChooser

        property int targetRow: -1

        onAccepted: {
            root.settings.paths.SetPathFromUrl(fileChooser.targetRow, selectedFile);
            fileChooser.targetRow = -1;
        }
    }

    FolderDialog {
        id: folderChooser

        property int targetRow: -1

        onAccepted: {
            root.settings.paths.SetPathFromUrl(folderChooser.targetRow, selectedFolder);
            folderChooser.targetRow = -1;
        }
    }

    FolderDialog {
        id: toolSuiteChooser

        title: qsTr("Locate Radeon Developer Tool Suite")

        onAccepted: root.settings.paths.LocateToolSuite(selectedFolder)
    }

    ColumnLayout {
        id: settingsColumn

        width: root.width - Theme.contentMargin * 2
        x: Theme.contentMargin
        y: Theme.contentMargin
        spacing: 14

        RdpLabel {
            text: qsTr("Settings")
            font.pointSize: 12
            font.bold: true
        }

        Repeater {
            model: root.settings.paths

            PathSettingRow {
                id: pathRow

                required property int index

                // The roles are read through the model object rather than as required
                // properties: PathSettingRow has its own label, path and status, and a
                // required property of the same name would shadow it and bind to itself.
                required property var model

                // The backend-test rows are only meaningful with RDP_ENABLE_RDF_BACKEND_TEST
                // set, and the settings-file row only exists in driver-settings builds.
                readonly property bool available: {
                    if (pathRow.index >= 4 && pathRow.index <= 6)
                        return root.settings.backendTestVisible;
                    if (pathRow.index === 7)
                        return root.settings.driverSettingsEnabled;
                    return true;
                }

                Layout.fillWidth: true
                visible: available
                // A hidden row must not reserve space in the column.
                Layout.preferredHeight: available ? implicitHeight : 0

                label: pathRow.model.label
                path: pathRow.model.path
                status: pathRow.model.status
                statusColor: Theme.statusError

                onPathEdited: newPath => root.settings.paths.SetPath(pathRow.index, newPath)
                onBrowseRequested: {
                    if (pathRow.model.isDirectory) {
                        folderChooser.targetRow = pathRow.index;
                        folderChooser.title = pathRow.model.browseTitle;
                        folderChooser.open();
                    } else {
                        fileChooser.targetRow = pathRow.index;
                        fileChooser.title = pathRow.model.browseTitle;
                        fileChooser.open();
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            RdpButton {
                text: qsTr("Locate Radeon Developer Tool Suite")
                onClicked: toolSuiteChooser.open()
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.topMargin: 6
            color: Theme.separator
        }

        RdpCheckBox {
            text: qsTr("Auto-open traces")
            checked: root.settings.autoOpenTraces
            onToggled: root.settings.autoOpenTraces = checked
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            RdpLabel {
                text: qsTr("Color Theme")
            }

            RdpComboBox {
                Layout.preferredWidth: 200
                // Ordered to match ColorThemeOption: Light, Dark, System.
                model: [qsTr("Light"), qsTr("Dark"), qsTr("Detect OS")]
                currentIndex: root.settings.colorTheme
                onActivated: root.settings.colorTheme = currentIndex
            }

            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Item {
                Layout.fillWidth: true
            }

            RdpButton {
                text: qsTr("Restore Defaults")
                onClicked: root.settings.RestoreDefaults()
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.contentMargin
        }
    }
}
