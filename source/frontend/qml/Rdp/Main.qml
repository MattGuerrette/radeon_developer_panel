pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme
import Rdp.Views

ApplicationWindow {
    id: window

    readonly property var pageNames: [qsTr("CONNECTION"), qsTr("CAPTURE"), qsTr("SYSTEM")]

    // Index 3 is the settings page, which the navigation bar's gear button opens but
    // which has no tab of its own.
    readonly property int settingsPage: 3

    property int currentPage: 0

    width: 1280
    height: 880
    minimumWidth: 940
    minimumHeight: 620
    visible: true
    color: Theme.window
    palette: Theme.controlPalette
    title: Backend.connected
           ? qsTr("%1 - Radeon Developer Panel %2").arg(Backend.connectionDescription || qsTr("Local")).arg(Backend.version)
           : qsTr("No connection - Radeon Developer Panel %1").arg(Backend.version)

    // Rdp.Theme deliberately does not depend on Rdp.Backend, so the saved theme is
    // pushed into it from here rather than read from there.
    Binding {
        target: Theme
        property: "colorTheme"
        value: Backend.settings.colorTheme
    }

    // Leaving the CAPTURE or SYSTEM page open through a disconnect would show panes with
    // nothing behind them, so fall back to CONNECTION.
    Connections {
        target: Backend

        function onConnectedChanged() {
            if (!Backend.connected && window.currentPage !== window.settingsPage) {
                window.currentPage = 0;
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        NavigationBar {
            Layout.fillWidth: true
            online: Backend.connected

            trailing: [
                RdpLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: window.currentPage === 2
                    text: qsTr("Changes have been made to driver experiments")
                    font.pointSize: 7.5
                    font.bold: true
                    color: Theme.statusWarning
                },
                RdpToolButton {
                    icon_: Icons.gear
                    selected: window.currentPage === window.settingsPage
                    onClicked: window.currentPage = window.settingsPage
                },
                RdpToolButton {
                    icon_: Icons.bug
                    onClicked: Backend.CopyBugReport()
                },
                RdpToolButton {
                    icon_: Icons.question
                    onClicked: Backend.OpenHelp()
                }
            ]

            Repeater {
                model: window.pageNames

                NavigationTab {
                    id: navigationTab

                    required property int index
                    required property string modelData

                    text: modelData
                    enabled: index === 0 || Backend.connected
                    selected: window.currentPage === index
                    onClicked: window.currentPage = navigationTab.index
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: window.currentPage

            ConnectionPage {}

            CapturePage {}

            SystemPage {}

            SettingsPage {}
        }
    }

    NotificationOverlay {
        anchors.fill: parent
    }
}
