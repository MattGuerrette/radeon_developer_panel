pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Rdp.Theme

// Stands in for the QTreeView-based tables in RDP: a style-drawn, resizable
// header section over a virtualised, alternating-row table. Clicking a header
// section sorts on that column and clicking it again flips the direction, the
// way QHeaderView does with setSortingEnabled().
Rectangle {
    id: root

    // [{ title: "Name", width: 200, flex: true, mono: false, color: <color>,
    //    sortable: false }, ...]
    property var columns: []
    // [[cell, cell, ...], ...]
    property var rows: []
    property int rowHeight: 20
    property int emptyRowCount: 0
    // Turns click-to-sort off for the whole table; individual columns can opt
    // out with "sortable": false in their descriptor.
    property bool sortable: true

    readonly property real fixedWidth: {
        let total = 0;
        for (const column of columns) {
            if (!column.flex)
                total += column.width;
        }
        return total;
    }
    readonly property int flexCount: {
        let count = 0;
        for (const column of columns) {
            if (column.flex)
                ++count;
        }
        return count;
    }

    color: Theme.base
    border.width: 1
    border.color: Theme.separator
    clip: true

    function columnWidth(column) {
        return column.flex
            ? Math.max(60, (root.width - 2 - root.fixedWidth) / Math.max(1, root.flexCount))
            : column.width;
    }

    function columnSortable(index) {
        const column = root.columns[index];
        return root.sortable && column !== undefined && column.sortable !== false;
    }

    // Flex column widths are measured against the table's own width, and
    // TableView caches what columnWidthProvider returns, so the layout has to be
    // re-run whenever that width moves. It goes through a zero-interval timer
    // because the width usually changes while TableView is already mid-layout,
    // and forceLayout() refuses to reenter one (Qt.callLater still lands inside
    // the same pass).
    onWidthChanged: relayout.restart()

    Timer {
        id: relayout

        interval: 0
        onTriggered: table.forceLayout()
    }

    RdpTableModel {
        id: tableModel

        rows: root.rows
        headers: root.columns.map(function (column) { return column.title; })
        placeholderRowCount: root.emptyRowCount
    }

    HorizontalHeaderView {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 1
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        clip: true
        syncView: table
        resizableColumns: true

        // Replaces the style's section so the sort indicator and the click
        // target live in the same item. Resizing still belongs to the header
        // view itself: the tap handler below yields to it as soon as a press
        // turns into a drag.
        delegate: Rectangle {
            id: section

            required property int column
            required property var display

            readonly property bool sortable: root.columnSortable(section.column)
            readonly property bool sorted: tableModel.sortColumn === section.column

            implicitWidth: root.columns[section.column] !== undefined
                ? root.columnWidth(root.columns[section.column])
                : 60
            implicitHeight: title.implicitHeight + 8
            color: section.sortable && sectionHover.hovered ? Theme.hover : Theme.alternateBase

            // Only the trailing edge, so adjacent sections share a single rule.
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: Theme.separator
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.separator
            }

            Text {
                id: title

                anchors.left: parent.left
                anchors.right: indicator.left
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: 6
                rightPadding: 2
                text: section.display === undefined ? "" : section.display
                font.pointSize: Theme.tablePointSize
                color: Theme.windowText
                elide: Text.ElideRight
                renderType: Text.NativeRendering
            }

            RdpIcon {
                id: indicator

                anchors.right: parent.right
                anchors.rightMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                width: 8
                height: 8
                visible: section.sorted
                icon: Icons.caretDown
                color: Theme.windowText
                // The caret points down for a descending sort and is flipped for
                // an ascending one, matching QStyle's header indicators.
                rotation: tableModel.sortOrder === Qt.AscendingOrder ? 180 : 0
            }

            HoverHandler {
                id: sectionHover
            }

            TapHandler {
                enabled: section.sortable
                // Releasing outside the section cancels, so a mis-aimed column
                // resize never lands as a sort.
                gesturePolicy: TapHandler.ReleaseWithinBounds
                onTapped: tableModel.ToggleSort(section.column)
            }
        }
    }

    TableView {
        id: table

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        anchors.bottomMargin: 1
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: tableModel
        columnWidthProvider: function (column) { return root.columnWidth(root.columns[column]); }
        rowHeightProvider: function (row) { return root.rowHeight; }

        ScrollBar.vertical: RdpScrollBar {}

        delegate: Rectangle {
            id: cell

            required property int row
            required property int column
            required property var display

            readonly property var descriptor: root.columns[cell.column]

            implicitWidth: root.columnWidth(cell.descriptor)
            implicitHeight: root.rowHeight
            color: cell.row % 2 === 0 ? Theme.base : Theme.alternateBase

            Text {
                anchors.fill: parent
                leftPadding: 6
                text: cell.display === undefined ? "" : cell.display
                font.pointSize: Theme.tablePointSize
                font.family: cell.descriptor.mono ? Theme.monoFamily : Application.font.family
                color: cell.descriptor.color !== undefined ? cell.descriptor.color
                                                           : Theme.windowText
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                renderType: Text.NativeRendering
            }
        }
    }
}
