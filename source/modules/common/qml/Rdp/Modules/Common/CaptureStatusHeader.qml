import QtQuick
import QtQuick.Layouts

import Rdp.Controls
import Rdp.Theme

// The status line at the top of every capture module's options column: what the
// module is doing, which process it is attached to, and why it cannot capture.
ColumnLayout {
    id: root

    required property var viewModel

    spacing: 2

    RowLayout {
        Layout.fillWidth: true
        spacing: 4

        RdpLabel {
            text: qsTr("Status:")
            font.bold: true
        }

        RdpLabel {
            Layout.fillWidth: true
            text: root.viewModel ? root.viewModel.statusText : ""
            font.bold: true
            // The view model picks the colour, the same one the Qt Widgets status label used.
            color: root.viewModel && root.viewModel.statusColor.a > 0
                   ? root.viewModel.statusColor
                   : Theme.windowText
            elide: Text.ElideRight
        }
    }

    RdpLabel {
        Layout.fillWidth: true
        visible: text.length > 0
        text: root.viewModel ? root.viewModel.connectedProcessText : ""
        font.pointSize: Theme.smallPointSize
        font.bold: true
        elide: Text.ElideRight
    }

    RdpLabel {
        Layout.fillWidth: true
        visible: text.length > 0
        text: root.viewModel ? root.viewModel.disabledReason : ""
        font.pointSize: Theme.smallPointSize
        color: Theme.statusWarning
        wrapMode: Text.WordWrap
    }
}
