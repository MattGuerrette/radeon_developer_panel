pragma ComponentBehavior: Bound

import QtQuick

import Rdp.Controls
import Rdp.Theme

// Hosts one DevTools module's view.
//
// This is the seam the Qt Widgets front-end had at DevToolsModule::CreateView(): the
// module names a QML component and an object for it to bind to, and the front-end knows
// nothing else about it. A module's view therefore lives with the module rather than here.
Item {
    id: root

    // The URL of the module's QML view, from ModuleModel's qmlViewUrl role.
    property url viewUrl

    // The object that view binds to, from ModuleModel's viewModel role. Named apart from
    // that role so a delegate can bind one to the other without shadowing itself.
    property var moduleViewModel

    Loader {
        id: loader

        anchors.fill: parent
        // Only built once the tab is actually looked at, and torn down with it.
        active: root.visible && root.viewUrl != ""
        asynchronous: true
        source: root.viewUrl

        onLoaded: item.viewModel = root.moduleViewModel
    }

    EmptyStateLabel {
        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 420)
        visible: root.viewUrl == ""
        text: qsTr("No view is available for this feature.")
    }
}
