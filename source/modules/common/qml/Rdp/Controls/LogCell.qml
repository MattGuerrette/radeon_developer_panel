import QtQuick

import Rdp.Theme

// One fixed-width, monospaced cell of the output log. The log is read far more
// often than it is measured, so the columns are fixed rather than elastic.
Text {
    leftPadding: 6
    rightPadding: 2
    height: parent.height
    font.pointSize: Theme.tablePointSize
    font.family: Theme.monoFamily
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
    renderType: Text.NativeRendering
}
