// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Declaration of the channel C++ uses to raise dialogs in the QML front-end.

#ifndef RDP_SOURCE_MODULES_COMMON_QML_CONTROLS_NOTIFICATIONS_H_
#define RDP_SOURCE_MODULES_COMMON_QML_CONTROLS_NOTIFICATIONS_H_

#include <functional>
#include <mutex>
#include <unordered_map>

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class QJSEngine;
class QQmlEngine;

namespace rdp
{
    /// @brief The channel C++ uses to raise a dialog in the QML front-end.
    ///
    /// Replaces qt_common's MessageOverlay, which is a QWidget. Anything in the front-end or
    /// in a module posts a message from any thread; the singleton re-emits it on the UI
    /// thread, where NotificationOverlay shows it. Questions carry a callback that fires
    /// once the user answers.
    class Notifications : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_SINGLETON

    public:
        /// @brief How prominently a message should be presented.
        enum Severity : int
        {
            kInfo = 0,   ///< Informational, dismissed with OK.
            kWarning,    ///< A warning, dismissed with OK.
            kCritical,   ///< An error, dismissed with OK.
            kQuestion    ///< A question, answered Yes or No.
        };
        Q_ENUM(Severity)

        /// @brief Gets the process-wide instance.
        /// @return The instance.
        static Notifications& Instance();

        /// @brief QML singleton factory.
        /// @return The instance for the QML engine.
        static Notifications* create(QQmlEngine*, QJSEngine*);

        /// @brief Raises an informational message.
        /// @param [in] title The dialog title.
        /// @param [in] text The dialog body.
        static void Info(const QString& title, const QString& text);

        /// @brief Raises a warning.
        /// @param [in] title The dialog title.
        /// @param [in] text The dialog body.
        static void Warning(const QString& title, const QString& text);

        /// @brief Raises an error.
        /// @param [in] title The dialog title.
        /// @param [in] text The dialog body.
        static void Critical(const QString& title, const QString& text);

        /// @brief Asks the user a yes/no question.
        /// @param [in] title The dialog title.
        /// @param [in] text The question.
        /// @param [in] on_answer Called on the UI thread with true when the user answered Yes.
        static void Question(const QString& title, const QString& text, std::function<void(bool)> on_answer);

        /// @brief Called by the front-end once the user has dismissed a message.
        /// @param [in] id The identifier the message was posted with.
        /// @param [in] accepted true when the user answered Yes.
        Q_INVOKABLE void Respond(qulonglong id, bool accepted);

    signals:
        /// @brief Emitted on the UI thread for every message posted.
        /// @param [in] id The identifier to pass back to Respond().
        /// @param [in] severity One of the Severity values.
        /// @param [in] title The dialog title.
        /// @param [in] text The dialog body.
        void posted(qulonglong id, int severity, const QString& title, const QString& text);

    private:
        /// @brief Constructor.
        Notifications() = default;

        /// @brief Posts a message, from any thread.
        /// @param [in] severity The severity of the message.
        /// @param [in] title The dialog title.
        /// @param [in] text The dialog body.
        /// @param [in] on_answer The callback for a question, may be empty.
        static void Post(Severity severity, const QString& title, const QString& text, std::function<void(bool)> on_answer);

        std::mutex                                              pending_mutex_;  ///< Guards the pending answer callbacks.
        std::unordered_map<qulonglong, std::function<void(bool)>> pending_;      ///< Answer callbacks, keyed by message id.
        qulonglong                                              next_id_ = 1;    ///< The id given to the next message posted.
    };
}  // namespace rdp

#endif
