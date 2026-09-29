pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Rdp.Theme

ComboBox {
    id: control

    implicitHeight: Theme.controlHeight
    leftPadding: 6
    rightPadding: 24
    font.pointSize: Theme.bodyPointSize

    indicator: RdpIcon {
        x: control.width - width - 8
        y: (control.height - height) / 2
        width: 9
        height: 9
        icon: Icons.chevronDown
        color: control.enabled ? Theme.windowText : Theme.disabledText
    }

    contentItem: Text {
        text: control.displayText
        font: control.font
        color: control.enabled ? Theme.text : Theme.disabledText
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        renderType: Text.NativeRendering
    }

    background: Rectangle {
        color: control.enabled ? Theme.controlBackground : Theme.window
        border.width: 1
        border.color: control.activeFocus ? Theme.accent
                                          : control.enabled ? Theme.controlBorder : Theme.mid
    }

    delegate: ItemDelegate {
        id: delegateItem

        required property int index
        required property var modelData

        width: control.width
        height: Theme.controlHeight
        highlighted: control.highlightedIndex === index

        contentItem: Text {
            text: delegateItem.modelData
            font: control.font
            color: Theme.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            renderType: Text.NativeRendering
        }

        background: Rectangle {
            color: delegateItem.highlighted ? Theme.highlight : Theme.base
        }
    }

    popup: Popup {
        y: control.height
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 2, 240)
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            ScrollBar.vertical: RdpScrollBar {}
        }

        background: Rectangle {
            color: Theme.base
            border.width: 1
            border.color: Theme.controlBorder
        }
    }
}
