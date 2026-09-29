import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import Rdp.Controls
import Rdp.Theme

// The capture hotkey. Pressing Edit puts the view model into listening mode; the next key
// press anywhere in the window becomes the hotkey.
//
// The key press is read in C++ rather than here because a global hotkey needs the
// platform's virtual key code, which Qt Quick's KeyEvent does not carry.
RowLayout {
    id: root

    // The module's capture view model, which owns the shortcut.
    required property var viewModel

    spacing: 4

    RdpTextField {
        Layout.fillWidth: true
        readOnly: true
        text: root.viewModel
              ? (root.viewModel.listeningForShortcut ? qsTr("Press a key combination...") : root.viewModel.captureShortcut)
              : ""
        color: root.viewModel && root.viewModel.listeningForShortcut ? Theme.accent : Theme.text
    }

    RdpIconButton {
        Layout.preferredWidth: 14
        Layout.preferredHeight: 14
        icon_: Icons.refreshCircle
        onClicked: root.viewModel.ResetShortcut()

        ToolTip.visible: hovered
        ToolTip.text: qsTr("Reset shortcut")
    }

    RdpIconButton {
        Layout.preferredWidth: 14
        Layout.preferredHeight: 14
        icon_: Icons.pencil
        onClicked: {
            if (root.viewModel.listeningForShortcut)
                root.viewModel.EndShortcutCapture();
            else
                root.viewModel.BeginShortcutCapture(Window.window);
        }

        ToolTip.visible: hovered
        ToolTip.text: qsTr("Edit shortcut")
    }
}
