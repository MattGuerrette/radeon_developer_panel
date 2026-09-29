// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of the channel C++ uses to raise dialogs in the QML front-end.

#include "notifications.h"

#include <utility>

#include <QJSEngine>
#include <QQmlEngine>

namespace rdp
{
    Notifications& Notifications::Instance()
    {
        static Notifications instance;
        return instance;
    }

    Notifications* Notifications::create(QQmlEngine*, QJSEngine*)
    {
        Notifications& instance = Instance();

        // The engine must not delete a function-local static.
        QJSEngine::setObjectOwnership(&instance, QJSEngine::CppOwnership);
        return &instance;
    }

    void Notifications::Info(const QString& title, const QString& text)
    {
        Post(kInfo, title, text, {});
    }

    void Notifications::Warning(const QString& title, const QString& text)
    {
        Post(kWarning, title, text, {});
    }

    void Notifications::Critical(const QString& title, const QString& text)
    {
        Post(kCritical, title, text, {});
    }

    void Notifications::Question(const QString& title, const QString& text, std::function<void(bool)> on_answer)
    {
        Post(kQuestion, title, text, std::move(on_answer));
    }

    void Notifications::Post(const Severity severity, const QString& title, const QString& text, std::function<void(bool)> on_answer)
    {
        Notifications& instance = Instance();

        qulonglong id = 0;
        {
            const std::lock_guard<std::mutex> lock(instance.pending_mutex_);
            id = instance.next_id_++;
            if (on_answer)
            {
                instance.pending_.emplace(id, std::move(on_answer));
            }
        }

        // Models post from the connection and module threads, so hop to the UI thread the
        // singleton lives on before touching the front-end.
        QMetaObject::invokeMethod(
            &instance,
            [&instance, id, severity, title, text] { emit instance.posted(id, static_cast<int>(severity), title, text); },
            Qt::QueuedConnection);
    }

    void Notifications::Respond(const qulonglong id, const bool accepted)
    {
        std::function<void(bool)> callback;
        {
            const std::lock_guard<std::mutex> lock(pending_mutex_);
            const auto                        it = pending_.find(id);
            if (it == pending_.end())
            {
                return;
            }

            callback = std::move(it->second);
            pending_.erase(it);
        }

        if (callback)
        {
            callback(accepted);
        }
    }
}  // namespace rdp
