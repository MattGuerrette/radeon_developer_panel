// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Definition for the model behind the RDP settings page.

#ifndef RDP_SOURCE_FRONTEND_MODELS_APP_SETTINGS_MODEL_H_
#define RDP_SOURCE_FRONTEND_MODELS_APP_SETTINGS_MODEL_H_

#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

namespace rdp
{
    class SettingsManager;

    /// @brief The tool paths shown on the settings page, with their validation state.
    ///
    /// Each row is one "path and filename to ..." line. The rows are fixed: they describe
    /// the tools RDP hands its captures to, and the model keeps each one's settings key,
    /// current value and whether that value currently points at something usable.
    class SettingsPathModel final : public QAbstractListModel
    {
        Q_OBJECT
        QML_ANONYMOUS

    public:
        /// @brief The roles the settings page binds to.
        enum Roles : int
        {
            kLabelRole = Qt::UserRole + 1,  ///< The prompt shown above the field.
            kPathRole,                      ///< The current path.
            kStatusRole,                    ///< Empty when the path is usable, otherwise why it is not.
            kValidRole,                     ///< Whether the path is usable.
            kIsDirectoryRole,               ///< Whether the row wants a directory rather than an executable.
            kBrowseTitleRole                ///< The title for the row's file chooser.
        };
        Q_ENUM(Roles)

        /// @brief Constructor.
        /// @param [in] settings_manager The application settings.
        /// @param [in] parent The parent object.
        explicit SettingsPathModel(std::shared_ptr<SettingsManager> settings_manager, QObject* parent = nullptr);

        /// @brief Destructor.
        ~SettingsPathModel() override;

        /// @brief QAbstractListModel::rowCount() override.
        /// @param [in] parent The parent index.
        /// @return The number of path rows.
        int rowCount(const QModelIndex& parent) const override;

        /// @brief QAbstractListModel::data() override.
        /// @param [in] index The index to query.
        /// @param [in] role The role to query.
        /// @return The data for the index and role.
        QVariant data(const QModelIndex& index, int role) const override;

        /// @brief QAbstractListModel::roleNames() override.
        /// @return The role names the settings page binds to.
        QHash<int, QByteArray> roleNames() const override;

        /// @brief Sets the path in the given row and writes it to the settings.
        /// @param [in] row The row to set.
        /// @param [in] path The new path.
        Q_INVOKABLE void SetPath(int row, const QString& path);

        /// @brief Sets the path in the given row from a file dialog's URL.
        /// @param [in] row The row to set.
        /// @param [in] url The chosen file or directory.
        Q_INVOKABLE void SetPathFromUrl(int row, const QUrl& url);

        /// @brief Puts every row back to the path RDP would have guessed on a fresh install.
        void RestoreDefaults();

        /// @brief Points every tool row at the executables inside a Radeon Developer Tool Suite folder.
        /// @param [in] url The tool suite directory.
        Q_INVOKABLE void LocateToolSuite(const QUrl& url);

        /// @brief Writes any missing rows into the settings and loads the saved values.
        void Load();

    private slots:
        /// @brief Re-checks every path, so a tool appearing on disk clears its warning.
        void ValidatePaths();

    private:
        /// @brief One "path and filename to ..." row.
        struct PathEntry
        {
            QString label;                  ///< The prompt shown above the field.
            QString settings_key;           ///< The key the path is stored under.
            QString browse_title;           ///< The title for the file chooser.
            QString invalid_text;           ///< Shown when the path is not usable.
            QString application_name;       ///< The public executable name, empty when not a tool path.
            QString internal_name;          ///< The internal executable name, empty when not a tool path.
            QString path;                   ///< The current path.
            bool    is_directory  = false;  ///< true when the row wants a directory.
            bool    is_tool_suite = false;  ///< true when "Locate Radeon Developer Tool Suite" fills the row in.
            bool    backend_test  = false;  ///< true when the row only appears with the backend-test UI enabled.
            bool    valid         = false;  ///< Whether the path currently points at something usable.
        };

        /// @brief Gets the path RDP guesses for a tool that has never been configured.
        /// @param [in] application_name The public executable name.
        /// @param [in] application_internal_name The internal executable name.
        /// @return The guessed path.
        static QString QueryDefaultPath(const QString& application_name, const QString& application_internal_name);

        /// @brief Gets the platform's default text editor path.
        /// @return The editor path.
        static QString GetDefaultTextEditorPath();

        /// @brief Gets the default path for a row.
        /// @param [in] entry The row.
        /// @return The default path.
        static QString DefaultPathFor(const PathEntry& entry);

        /// @brief Checks whether a row's path points at something usable.
        /// @param [in] entry The row to check.
        /// @return true when the path is usable.
        static bool IsPathValid(const PathEntry& entry);

        /// @brief Writes a row's path into the application settings.
        /// @param [in] entry The row to write.
        void Store(const PathEntry& entry) const;

        std::vector<PathEntry>           entries_;           ///< The path rows, in the order the page shows them.
        std::shared_ptr<SettingsManager> settings_manager_;  ///< The application settings.
        QTimer                           validation_timer_;  ///< Drives the periodic re-check of every path.
    };

    /// @brief The model behind the RDP settings page.
    class AppSettingsModel final : public QObject
    {
        Q_OBJECT
        QML_ANONYMOUS

        Q_PROPERTY(rdp::SettingsPathModel* paths READ Paths CONSTANT)
        Q_PROPERTY(bool autoOpenTraces READ AutoOpenTraces WRITE SetAutoOpenTraces NOTIFY autoOpenTracesChanged)
        Q_PROPERTY(int colorTheme READ ColorTheme WRITE SetColorTheme NOTIFY colorThemeChanged)
        Q_PROPERTY(bool backendTestVisible READ IsBackendTestVisible CONSTANT)
        Q_PROPERTY(bool driverSettingsEnabled READ IsDriverSettingsEnabled CONSTANT)

    public:
        /// @brief Constructor.
        /// @param [in] settings_manager The application settings.
        /// @param [in] parent The parent object.
        explicit AppSettingsModel(std::shared_ptr<SettingsManager> settings_manager, QObject* parent = nullptr);

        /// @brief Destructor.
        ~AppSettingsModel() override;

        /// @brief Gets the tool path rows.
        /// @return The path model.
        [[nodiscard]] SettingsPathModel* Paths() const;

        /// @brief Returns whether RDP opens captures as soon as they finish.
        /// @return true when auto-open is on.
        [[nodiscard]] bool AutoOpenTraces() const;

        /// @brief Sets whether RDP opens captures as soon as they finish.
        /// @param [in] enabled true to open captures automatically.
        void SetAutoOpenTraces(bool enabled);

        /// @brief Gets the selected colour theme, as a ColorThemeOption.
        /// @return The colour theme.
        [[nodiscard]] int ColorTheme() const;

        /// @brief Sets the colour theme and applies it.
        /// @param [in] theme The theme, as a ColorThemeOption.
        void SetColorTheme(int theme);

        /// @brief Returns whether the RDF backend-test paths should be shown.
        /// @return true when the backend-test UI is enabled.
        [[nodiscard]] static bool IsBackendTestVisible();

        /// @brief Returns whether the driver settings file path should be shown.
        /// @return true when driver settings are compiled in.
        [[nodiscard]] static bool IsDriverSettingsEnabled();

        /// @brief Puts every setting on the page back to its default.
        Q_INVOKABLE void RestoreDefaults();

    signals:
        /// @brief Emitted when the auto-open setting changes.
        void autoOpenTracesChanged();

        /// @brief Emitted when the colour theme changes.
        void colorThemeChanged();

    private:
        std::unique_ptr<SettingsPathModel> paths_;             ///< The tool path rows.
        std::shared_ptr<SettingsManager>   settings_manager_;  ///< The application settings.
    };
}  // namespace rdp

#endif
