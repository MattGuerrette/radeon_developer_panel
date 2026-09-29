// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the model behind the RDP settings page.

#include "models/app_settings_model.h"

#include <utility>

#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <qt_common/utils/qt_util.h>

#include "definitions.h"
#include "settings_manager.h"

namespace rdp
{
    namespace
    {
#ifdef Q_OS_WINDOWS
        constexpr const char* kRgpApplicationName                    = "RadeonGPUProfiler.exe";
        constexpr const char* kRgpApplicationNameInternal            = "RadeonGPUProfiler-Internal.exe";
        constexpr const char* kRmvApplicationName                    = "RadeonMemoryVisualizer.exe";
        constexpr const char* kRmvApplicationNameInternal            = "RadeonMemoryVisualizer-Internal.exe";
        constexpr const char* kRraApplicationName                    = "RadeonRaytracingAnalyzer.exe";
        constexpr const char* kRraApplicationNameInternal            = "RadeonRaytracingAnalyzer-Internal.exe";
        constexpr const char* kRgdApplicationName                    = "rgd.exe";
        constexpr const char* kRgdApplicationNameInternal            = "rgd-Internal.exe";
        constexpr const char* kRgpBackendTestApplicationName         = "RgpBackendTest.exe";
        constexpr const char* kRgpBackendTestApplicationNameInternal = "RgpBackendTest-Internal.exe";
        constexpr const char* kRmvBackendTestApplicationName         = "RmvBackendTest.exe";
        constexpr const char* kRmvBackendTestApplicationNameInternal = "RmvBackendTest-Internal.exe";
        constexpr const char* kRraBackendTestApplicationName         = "RraBackendTest.exe";
        constexpr const char* kRraBackendTestApplicationNameInternal = "RraBackendTest-Internal.exe";
#else
        constexpr const char* kRgpApplicationName                    = "RadeonGPUProfiler";
        constexpr const char* kRgpApplicationNameInternal            = "RadeonGPUProfiler-Internal";
        constexpr const char* kRmvApplicationName                    = "RadeonMemoryVisualizer";
        constexpr const char* kRmvApplicationNameInternal            = "RadeonMemoryVisualizer-Internal";
        constexpr const char* kRraApplicationName                    = "RadeonRaytracingAnalyzer";
        constexpr const char* kRraApplicationNameInternal            = "RadeonRaytracingAnalyzer-Internal";
        constexpr const char* kRgdApplicationName                    = "rgd";
        constexpr const char* kRgdApplicationNameInternal            = "rgd-Internal";
        constexpr const char* kRgpBackendTestApplicationName         = "RgpBackendTest";
        constexpr const char* kRgpBackendTestApplicationNameInternal = "RgpBackendTest-Internal";
        constexpr const char* kRmvBackendTestApplicationName         = "RmvBackendTest";
        constexpr const char* kRmvBackendTestApplicationNameInternal = "RmvBackendTest-Internal";
        constexpr const char* kRraBackendTestApplicationName         = "RraBackendTest";
        constexpr const char* kRraBackendTestApplicationNameInternal = "RraBackendTest-Internal";
#endif

        constexpr const char* kRgpExecutablePathKey             = "rgp_path";
        constexpr const char* kRmvExecutablePathKey             = "rmv_path";
        constexpr const char* kRraExecutablePathKey             = "rra_path";
        constexpr const char* kRgdExecutablePathKey             = "rgd_path";
        constexpr const char* kRgpBackendTestExecutablePathKey  = "rgp_backend_test_path";
        constexpr const char* kRmvBackendTestExecutablePathKey  = "rmv_backend_test_path";
        constexpr const char* kRraBackendTestExecutablePathKey  = "rra_backend_test_path";
        constexpr const char* kSettingsFilePathKey              = "settings_file_path";
        constexpr const char* kTxtEditorExecutablePathKey       = "txt_editor_path";
        constexpr const char* kAutoOpenTracesPathKey            = "auto_open_traces";

        constexpr int kValidationTime = 200;  ///< How often the paths are re-checked, in milliseconds.
    }  // namespace

    SettingsPathModel::SettingsPathModel(std::shared_ptr<SettingsManager> settings_manager, QObject* parent)
        : QAbstractListModel(parent)
        , settings_manager_(std::move(settings_manager))
    {
        entries_ = {
            {QObject::tr("Path and filename to Radeon GPU Profiler:"),
             kRgpExecutablePathKey,
             QObject::tr("Choose path to Radeon GPU Profiler..."),
             QObject::tr("Invalid RGP Executable"),
             kRgpApplicationName,
             kRgpApplicationNameInternal,
             {},
             false,
             true,
             false,
             false},
            {QObject::tr("Path and filename to Radeon Memory Visualizer:"),
             kRmvExecutablePathKey,
             QObject::tr("Choose path to Radeon Memory Visualizer..."),
             QObject::tr("Invalid RMV Executable"),
             kRmvApplicationName,
             kRmvApplicationNameInternal,
             {},
             false,
             true,
             false,
             false},
            {QObject::tr("Path and filename to Radeon Raytracing Analyzer:"),
             kRraExecutablePathKey,
             QObject::tr("Choose path to Radeon Raytracing Analyzer..."),
             QObject::tr("Invalid RRA Executable"),
             kRraApplicationName,
             kRraApplicationNameInternal,
             {},
             false,
             true,
             false,
             false},
            {QObject::tr("Path and filename to Radeon GPU Detective:"),
             kRgdExecutablePathKey,
             QObject::tr("Choose path to Radeon GPU Detective..."),
             QObject::tr("Invalid RGD Executable"),
             kRgdApplicationName,
             kRgdApplicationNameInternal,
             {},
             false,
             true,
             false,
             false},
            {QObject::tr("Path and filename to RGP Backend Test:"),
             kRgpBackendTestExecutablePathKey,
             QObject::tr("Choose path to RGP Backend Test executable..."),
             QObject::tr("Invalid RGP Backend Test Executable"),
             kRgpBackendTestApplicationName,
             kRgpBackendTestApplicationNameInternal,
             {},
             false,
             true,
             true,
             false},
            {QObject::tr("Path and filename to RMV Backend Test:"),
             kRmvBackendTestExecutablePathKey,
             QObject::tr("Choose path to RMV Backend Test executable..."),
             QObject::tr("Invalid RMV Backend Test Executable"),
             kRmvBackendTestApplicationName,
             kRmvBackendTestApplicationNameInternal,
             {},
             false,
             true,
             true,
             false},
            {QObject::tr("Path and filename to RRA Backend Test:"),
             kRraBackendTestExecutablePathKey,
             QObject::tr("Choose path to RRA Backend Test executable..."),
             QObject::tr("Invalid RRA Backend Test Executable"),
             kRraBackendTestApplicationName,
             kRraBackendTestApplicationNameInternal,
             {},
             false,
             true,
             true,
             false},
            {QObject::tr("Path to settings config file:"),
             kSettingsFilePathKey,
             QObject::tr("Choose root folder for settings file..."),
             QObject::tr("Invalid settings file path"),
             {},
             {},
             {},
             true,
             false,
             false,
             false},
            {QObject::tr("Path and filename to preferred text editor:"),
             kTxtEditorExecutablePathKey,
             QObject::tr("Choose path to preferred text editor..."),
             QObject::tr("Invalid text editor"),
             {},
             {},
             {},
             false,
             false,
             false,
             false},
        };

        connect(&validation_timer_, &QTimer::timeout, this, &SettingsPathModel::ValidatePaths);
    }

    SettingsPathModel::~SettingsPathModel() = default;

    QString SettingsPathModel::QueryDefaultPath(const QString& application_name, const QString& application_internal_name)
    {
        // A public build ships next to the public executable and an internal one next to
        // the internal executable, so prefer whichever is actually there.
        const QString   public_path   = QDir::toNativeSeparators(QString(".") + QDir::separator() + application_name);
        const QString   internal_path = QDir::toNativeSeparators(QString(".") + QDir::separator() + application_internal_name);
        const QFileInfo public_info(public_path);
        const QFileInfo internal_info(internal_path);

        if (public_info.exists())
        {
            return public_path;
        }

        if (internal_info.exists())
        {
            return internal_path;
        }

        return public_path;
    }

    QString SettingsPathModel::GetDefaultTextEditorPath()
    {
#ifdef Q_OS_WINDOWS
        return "C:\\Windows\\notepad.exe";
#else
        return "/usr/bin/gnome-text-editor";
#endif
    }

    QString SettingsPathModel::DefaultPathFor(const PathEntry& entry)
    {
        if (!entry.application_name.isEmpty())
        {
            return QueryDefaultPath(entry.application_name, entry.internal_name);
        }

        if (entry.settings_key == kTxtEditorExecutablePathKey)
        {
            return GetDefaultTextEditorPath();
        }

        return {};
    }

    bool SettingsPathModel::IsPathValid(const PathEntry& entry)
    {
        const QFileInfo file_info(entry.path);

        if (entry.is_directory)
        {
            return file_info.exists() && file_info.isDir();
        }

        return file_info.exists() && file_info.isExecutable() && !file_info.isDir();
    }

    void SettingsPathModel::Store(const PathEntry& entry) const
    {
        QSettings* settings = settings_manager_->GetSettings();
        if (settings != nullptr)
        {
            settings->setValue(entry.settings_key, entry.path);
            settings->sync();
        }
    }

    void SettingsPathModel::Load()
    {
        QSettings* settings = settings_manager_->GetSettings();
        Q_ASSERT(settings != nullptr);
        if (settings == nullptr)
        {
            return;
        }

        beginResetModel();

        for (PathEntry& entry : entries_)
        {
            if (settings->value(entry.settings_key).isNull())
            {
                settings->setValue(entry.settings_key, DefaultPathFor(entry));
            }

            entry.path  = QDir::toNativeSeparators(settings->value(entry.settings_key).toString());
            entry.valid = IsPathValid(entry);
        }

        // AMD_CONFIG_DIR overrides whatever was saved for the settings file location.
        if (const char* env_var = getenv("AMD_CONFIG_DIR"); env_var != nullptr)
        {
            for (PathEntry& entry : entries_)
            {
                if (entry.settings_key == kSettingsFilePathKey)
                {
                    entry.path = QDir::toNativeSeparators(QString::fromUtf8(env_var));
                    settings->setValue(entry.settings_key, entry.path);
                    entry.valid = IsPathValid(entry);
                }
            }
        }

        settings->sync();

        endResetModel();

        validation_timer_.start(kValidationTime);
    }

    int SettingsPathModel::rowCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(entries_.size());
    }

    QVariant SettingsPathModel::data(const QModelIndex& index, const int role) const
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(entries_.size()))
        {
            return {};
        }

        const PathEntry& entry = entries_[static_cast<size_t>(index.row())];

        switch (role)
        {
        case Qt::DisplayRole:
        case kLabelRole:
            return entry.label;
        case kPathRole:
            return entry.path;
        case kStatusRole:
            return entry.valid ? QString() : entry.invalid_text;
        case kValidRole:
            return entry.valid;
        case kIsDirectoryRole:
            return entry.is_directory;
        case kBrowseTitleRole:
            return entry.browse_title;
        default:
            return {};
        }
    }

    QHash<int, QByteArray> SettingsPathModel::roleNames() const
    {
        return {{kLabelRole, "label"},
                {kPathRole, "path"},
                {kStatusRole, "status"},
                {kValidRole, "valid"},
                {kIsDirectoryRole, "isDirectory"},
                {kBrowseTitleRole, "browseTitle"}};
    }

    void SettingsPathModel::SetPath(const int row, const QString& path)
    {
        if (row < 0 || row >= static_cast<int>(entries_.size()))
        {
            return;
        }

        PathEntry& entry = entries_[static_cast<size_t>(row)];
        if (entry.path == path)
        {
            return;
        }

        entry.path  = path;
        entry.valid = IsPathValid(entry);
        Store(entry);

        const QModelIndex changed = index(row, 0, {});
        emit dataChanged(changed, changed, {kPathRole, kStatusRole, kValidRole});
    }

    void SettingsPathModel::SetPathFromUrl(const int row, const QUrl& url)
    {
        if (url.isEmpty())
        {
            return;
        }

        SetPath(row, QDir::toNativeSeparators(url.toLocalFile()));
    }

    void SettingsPathModel::RestoreDefaults()
    {
        for (size_t i = 0; i < entries_.size(); i++)
        {
            SetPath(static_cast<int>(i), DefaultPathFor(entries_[i]));
        }
    }

    void SettingsPathModel::LocateToolSuite(const QUrl& url)
    {
        if (url.isEmpty())
        {
            return;
        }

        const QDir suite_dir(url.toLocalFile());
        if (suite_dir.isEmpty())
        {
            return;
        }

        for (size_t i = 0; i < entries_.size(); i++)
        {
            const PathEntry& entry = entries_[i];
            if (entry.is_tool_suite)
            {
                SetPath(static_cast<int>(i), QDir::toNativeSeparators(suite_dir.absoluteFilePath(entry.application_name)));
            }
        }
    }

    void SettingsPathModel::ValidatePaths()
    {
        for (size_t i = 0; i < entries_.size(); i++)
        {
            PathEntry& entry = entries_[i];

            if (const bool valid = IsPathValid(entry); valid != entry.valid)
            {
                entry.valid = valid;

                const QModelIndex changed = index(static_cast<int>(i), 0, {});
                emit dataChanged(changed, changed, {kStatusRole, kValidRole});
            }
        }
    }

    AppSettingsModel::AppSettingsModel(std::shared_ptr<SettingsManager> settings_manager, QObject* parent)
        : QObject(parent)
        , settings_manager_(std::move(settings_manager))
    {
        paths_ = std::make_unique<SettingsPathModel>(settings_manager_);

        QSettings* settings = settings_manager_->GetSettings();
        if (settings != nullptr)
        {
            if (settings->value(kAutoOpenTracesPathKey).isNull())
            {
                settings->setValue(kAutoOpenTracesPathKey, false);
            }

            if (settings->value(kApplicationSettingsThemeKey).isNull())
            {
                settings->setValue(kApplicationSettingsThemeKey, static_cast<int>(ColorThemeOption::kSystem));
            }

            settings->sync();
        }

        paths_->Load();

        settings_manager_->ApplyColorThemeOption(static_cast<ColorThemeOption>(ColorTheme()));
    }

    AppSettingsModel::~AppSettingsModel() = default;

    SettingsPathModel* AppSettingsModel::Paths() const
    {
        return paths_.get();
    }

    bool AppSettingsModel::AutoOpenTraces() const
    {
        const QSettings* settings = settings_manager_->GetSettings();
        return settings != nullptr && settings->value(kAutoOpenTracesPathKey, false).toBool();
    }

    void AppSettingsModel::SetAutoOpenTraces(const bool enabled)
    {
        if (enabled == AutoOpenTraces())
        {
            return;
        }

        if (QSettings* settings = settings_manager_->GetSettings(); settings != nullptr)
        {
            settings->setValue(kAutoOpenTracesPathKey, enabled);
            settings->sync();
        }

        emit autoOpenTracesChanged();
    }

    int AppSettingsModel::ColorTheme() const
    {
        const QSettings* settings = settings_manager_->GetSettings();
        if (settings == nullptr)
        {
            return static_cast<int>(ColorThemeOption::kSystem);
        }

        return settings->value(kApplicationSettingsThemeKey, static_cast<int>(ColorThemeOption::kSystem)).toInt();
    }

    void AppSettingsModel::SetColorTheme(const int theme)
    {
        const auto option = static_cast<ColorThemeOption>(theme);
        if (option == settings_manager_->GetColorThemeOption() && theme == ColorTheme())
        {
            return;
        }

        if (QSettings* settings = settings_manager_->GetSettings(); settings != nullptr)
        {
            settings->setValue(kApplicationSettingsThemeKey, theme);
            settings->sync();
        }

        settings_manager_->ApplyColorThemeOption(option);

        emit colorThemeChanged();
    }

    bool AppSettingsModel::IsBackendTestVisible()
    {
        return qEnvironmentVariableIsSet("RDP_ENABLE_RDF_BACKEND_TEST");
    }

    bool AppSettingsModel::IsDriverSettingsEnabled()
    {
#ifdef DRIVER_SETTINGS_ENABLED
        return true;
#else
        return false;
#endif
    }

    void AppSettingsModel::RestoreDefaults()
    {
        paths_->RestoreDefaults();
        SetAutoOpenTraces(false);
        SetColorTheme(static_cast<int>(ColorThemeOption::kSystem));
    }
}  // namespace rdp
