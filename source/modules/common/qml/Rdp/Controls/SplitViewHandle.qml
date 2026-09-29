import QtQuick
import QtQuick.Controls

import Rdp.Theme

// Named SplitViewHandle rather than SplitHandle so it does not shadow the
// SplitHandle attached type it reads its pressed/hovered state from.
Rectangle {
    id: root

    required property int orientation

    readonly property bool horizontal: orientation === Qt.Horizontal

    implicitWidth: horizontal ? 5 : 0
    implicitHeight: horizontal ? 0 : 5
    color: SplitHandle.pressed ? Theme.accentDim
                               : SplitHandle.hovered ? Theme.mid : Theme.window

    Column {
        anchors.centerIn: parent
        spacing: 2
        visible: root.horizontal

        Repeater {
            model: 4

            Rectangle {
                width: 1
                height: 1
                color: Theme.dark
            }
        }
    }

    Row {
        anchors.centerIn: parent
        spacing: 2
        visible: !root.horizontal

        Repeater {
            model: 4

            Rectangle {
                width: 1
                height: 1
                color: Theme.dark
            }
        }
    }
}
