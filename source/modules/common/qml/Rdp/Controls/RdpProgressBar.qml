import QtQuick
import QtQuick.Controls

import Rdp.Theme

ProgressBar {
    id: control

    implicitHeight: 18

    background: Rectangle {
        color: Theme.base
        border.width: 1
        border.color: Theme.controlBorder
    }

    contentItem: Item {
        clip: true

        // A determinate bar fills from the left; an indeterminate one sweeps a block
        // across, which is how a stage of unknown length is shown.
        Rectangle {
            id: fill

            width: control.indeterminate ? parent.width * 0.3 : control.visualPosition * parent.width
            height: parent.height
            color: Theme.iconAccent

            NumberAnimation on x {
                running: control.indeterminate && control.visible
                loops: Animation.Infinite
                from: -fill.width
                to: control.width
                duration: 1200
            }
        }

        // The sweep leaves x wherever it stopped, so it is put back for a determinate bar.
        Binding {
            target: fill
            property: "x"
            value: 0
            when: !control.indeterminate
        }
    }
}
