import QtQuick
import QtQuick.Layouts

import Rdp.Theme

// One "path and filename to ..." line on the settings page: a prompt, an editable
// path with a browse button, and a status line that is only occupied while the
// path does not point at anything usable.
ColumnLayout {
    id: root

    property string label
    property string path
    property string status
    property color statusColor: Theme.statusGood

    signal pathEdited(string newPath)
    signal browseRequested

    spacing: 4

    RdpLabel {
        Layout.fillWidth: true
        text: root.label
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 6

        RdpTextField {
            id: field

            Layout.fillWidth: true
            text: root.path

            // The model rewrites the path on its own (Restore Defaults, Locate Tool
            // Suite, an external edit), so the field tracks it except while the user
            // is the one typing.
            onTextEdited: root.pathEdited(text)

            Connections {
                target: root

                function onPathChanged() {
                    if (!field.activeFocus)
                        field.text = root.path;
                }
            }
        }

        RdpButton {
            Layout.preferredWidth: 34
            leftPadding: 4
            rightPadding: 4
            text: qsTr("...")
            onClicked: root.browseRequested()
        }
    }

    RdpLabel {
        Layout.fillWidth: true
        visible: root.status.length > 0
        text: root.status
        font.pointSize: Theme.smallPointSize
        color: root.statusColor
    }
}
