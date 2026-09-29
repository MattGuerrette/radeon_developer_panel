import QtQuick

import Rdp.Theme

// Renders an entry from the Icons singleton, tinted to the current theme.
// The path data is wrapped in an inline SVG document rather than drawn with
// Shape/ShapePath, because Shape render nodes are not clipped by an enclosing
// Flickable and leak outside scrolling panes.
Item {
    id: root

    property var icon
    property color color: Theme.windowText

    readonly property real strokeWidth: (icon && icon.strokeWidth) ? icon.strokeWidth : 0
    readonly property string paint: strokeWidth > 0
        ? 'fill="none" stroke="' + color + '" stroke-width="' + strokeWidth
          + '" stroke-linecap="square" stroke-miterlimit="10"'
        : 'fill="' + color + '"'

    implicitWidth: 16
    implicitHeight: 16

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        mipmap: true
        sourceSize.width: Math.max(1, Math.round(root.width))
        sourceSize.height: Math.max(1, Math.round(root.height))
        source: root.icon
                ? "data:image/svg+xml;utf8," + encodeURIComponent(
                      '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 '
                      + root.icon.width + ' ' + root.icon.height + '"><path '
                      + root.paint + ' d="' + root.icon.path + '"/></svg>')
                : ""
    }
}
