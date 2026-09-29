import QtQuick

import QtQuick.Controls

import Rdp.Theme

Label {
    color: enabled ? Theme.windowText : Theme.disabledText
    font.pointSize: Theme.bodyPointSize
    renderType: Text.NativeRendering
}
