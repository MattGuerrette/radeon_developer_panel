pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

ColumnLayout {
    id: root

    readonly property var log: Backend.log

    spacing: 8

    RowLayout {
        Layout.fillWidth: true
        spacing: 6

        RdpLabel {
            text: qsTr("Source:")
        }

        RdpComboBox {
            Layout.preferredWidth: 160
            model: root.log.sourceFilters
            textRole: "name"
            currentIndex: root.log.sourceFilterIndex
            onActivated: root.log.sourceFilterIndex = currentIndex
        }

        RdpLabel {
            Layout.leftMargin: 10
            text: qsTr("PID:")
        }

        RdpComboBox {
            Layout.preferredWidth: 160
            model: root.log.pidFilters
            textRole: "name"
            currentIndex: root.log.pidFilterIndex
            onActivated: root.log.pidFilterIndex = currentIndex
        }

        RdpLabel {
            Layout.leftMargin: 10
            text: qsTr("UMD:")
        }

        RdpComboBox {
            Layout.preferredWidth: 120
            // Only meaningful once a single process has been picked.
            enabled: root.log.umdFilterEnabled
            model: root.log.umdIdFilters
            textRole: "name"
            currentIndex: root.log.umdIdFilterIndex
            onActivated: root.log.umdIdFilterIndex = currentIndex
        }

        Item {
            Layout.fillWidth: true
        }

        RdpButton {
            visible: root.log.hasLogFile
            text: qsTr("Open log file location")
            onClicked: root.log.OpenLogFileLocation()
        }
    }

    // The log is long-lived and appended to continuously, so it is presented straight
    // from the model rather than copied into DataTable's row arrays.
    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: Theme.base
        border.width: 1
        border.color: Theme.separator
        clip: true

        ListView {
            id: entries

            anchors.fill: parent
            anchors.margins: 1
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: root.log.entries

            ScrollBar.vertical: RdpScrollBar {}

            // Follow the tail, unless the user has scrolled away from it.
            property bool atBottom: atYEnd

            onCountChanged: {
                if (entries.atBottom)
                    entries.positionViewAtEnd();
            }

            delegate: Rectangle {
                id: entry

                required property int index
                required property string timestamp
                required property int level
                required property string levelName
                required property string source
                required property string pid
                required property string message

                // Mirrors the colours the Qt Widgets log delegate painted rows in.
                readonly property color levelColor: {
                    switch (entry.level) {
                    case 3:
                        return Theme.statusError;
                    case 2:
                        return Theme.statusWarning;
                    default:
                        return Theme.windowText;
                    }
                }

                width: ListView.view.width
                height: 18
                color: entry.index % 2 === 0 ? Theme.base : Theme.alternateBase

                Row {
                    anchors.fill: parent

                    LogCell {
                        width: 110
                        text: entry.timestamp
                        color: entry.levelColor
                    }

                    LogCell {
                        width: 70
                        text: entry.levelName
                        color: entry.levelColor
                    }

                    LogCell {
                        width: 110
                        text: entry.source
                        color: entry.levelColor
                    }

                    LogCell {
                        width: 70
                        text: entry.pid
                        color: entry.levelColor
                    }

                    LogCell {
                        width: Math.max(0, entry.width - 360)
                        text: entry.message
                        color: entry.levelColor
                    }
                }
            }
        }

        EmptyStateLabel {
            anchors.centerIn: parent
            visible: entries.count === 0
            text: qsTr("Nothing has been logged yet.")
        }
    }
}
