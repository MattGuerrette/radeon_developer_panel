import QtQuick

import Rdp.Modules.Common

// The Crash Analysis module's view. The front-end instantiates this and sets viewModel to the
// module's CaptureViewModel.
//
// Everything the capture modules have in common -- status, capture controls, progress,
// output path and the list of captured files -- comes from CaptureModuleView. The panes
// below it are Crash Analysis's own settings.
CaptureModuleView {
    id: root

    moduleName: qsTr("Crash Analysis")
}
