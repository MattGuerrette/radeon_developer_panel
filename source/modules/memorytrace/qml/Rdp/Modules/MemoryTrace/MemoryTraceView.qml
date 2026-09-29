import QtQuick

import Rdp.Modules.Common

// The Memory Trace module's view. The front-end instantiates this and sets viewModel to the
// module's CaptureViewModel.
//
// Everything the capture modules have in common -- status, capture controls, progress,
// output path and the list of captured files -- comes from CaptureModuleView. The panes
// below it are Memory Trace's own settings.
CaptureModuleView {
    id: root

    moduleName: qsTr("Memory Trace")
}
