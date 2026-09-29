pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Rdp.Backend
import Rdp.Controls
import Rdp.Theme

// Shows the messages the C++ models post through Notifications, one at a time.
// Replaces qt_common's MessageOverlay, which is a QWidget. Messages that arrive
// while one is already up are queued rather than dropped, so a burst during
// connection does not lose the last one.
Item {
    id: root

    property var queue: []
    property var current: null

    function present() {
        if (root.current !== null || root.queue.length === 0)
            return;

        root.current = root.queue[0];
        root.queue = root.queue.slice(1);
        dialog.open();
    }

    function dismiss(accepted) {
        if (root.current === null)
            return;

        Notifications.Respond(root.current.id, accepted);
        root.current = null;
        root.present();
    }

    Connections {
        target: Notifications

        function onPosted(id, severity, title, text) {
            root.queue = root.queue.concat([
                {
                    "id": id,
                    "severity": severity,
                    "title": title,
                    "text": text
                }
            ]);
            root.present();
        }
    }

    Dialog {
        id: dialog

        readonly property bool isQuestion: root.current !== null
                                           && root.current.severity === Notifications.kQuestion

        anchors.centerIn: parent
        width: Math.min(520, root.width - 80)
        modal: true
        closePolicy: Popup.NoAutoClose
        title: root.current === null ? "" : root.current.title
        standardButtons: isQuestion
                         ? (Dialog.Yes | Dialog.No)
                         : Dialog.Ok

        onAccepted: root.dismiss(true)
        onRejected: root.dismiss(false)

        contentItem: RowLayout {
            spacing: 12

            RdpIcon {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                icon: dialog.isQuestion ? Icons.question : Icons.warning
                color: {
                    if (root.current === null)
                        return Theme.iconMuted;
                    switch (root.current.severity) {
                    case Notifications.kCritical:
                        return Theme.statusError;
                    case Notifications.kWarning:
                        return Theme.statusWarning;
                    default:
                        return Theme.iconAccent;
                    }
                }
            }

            RdpLabel {
                Layout.fillWidth: true
                text: root.current === null ? "" : root.current.text
                wrapMode: Text.WordWrap
            }
        }
    }
}
