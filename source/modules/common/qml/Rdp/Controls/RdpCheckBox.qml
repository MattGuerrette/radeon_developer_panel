import QtQuick
import QtQuick.Controls

import Rdp.Theme

CheckBox {
    id: control

    spacing: 6
    font.pointSize: Theme.bodyPointSize

    // The label is offset by padding on the control rather than inside the content item,
    // so the content item is handed the whole width it is allowed to wrap in. Putting the
    // offset inside the text instead leaves the control one line tall while the text draws
    // two, and the second line then falls outside the control's clickable area.
    leftPadding: indicator.implicitWidth + spacing
    topPadding: 2
    bottomPadding: 2

    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: Math.max(indicator.implicitHeight, implicitContentHeight) + topPadding + bottomPadding

    indicator: Rectangle {
        implicitWidth: 14
        implicitHeight: 14
        x: 0
        y: (control.height - height) / 2
        color: control.enabled ? Theme.controlBackground : Theme.window
        border.width: 1
        border.color: control.enabled ? Theme.controlBorder : Theme.mid

        RdpIcon {
            anchors.centerIn: parent
            width: 9
            height: 9
            visible: control.checkState !== Qt.Unchecked
            icon: Icons.checkmark
            color: control.enabled ? Theme.windowText : Theme.disabledText
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.enabled ? Theme.windowText : Theme.disabledText
        verticalAlignment: Text.AlignVCenter
        // Some of the capture settings are labelled with a sentence and the panes they sit
        // in are only as wide as the sidebar, so the label wraps inside its pane.
        wrapMode: Text.WordWrap
        renderType: Text.NativeRendering
    }
}
