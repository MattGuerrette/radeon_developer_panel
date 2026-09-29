// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Declaration for the QML-facing wrapper around a trace source view model.

#ifndef RDP_SOURCE_MODULES_COMMON_INC_MODEL_CAPTURE_VIEW_MODEL_H_
#define RDP_SOURCE_MODULES_COMMON_INC_MODEL_CAPTURE_VIEW_MODEL_H_

#include <functional>
#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QColor>
#include <QDateTime>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include "trace_source_view_model.h"
#include "utility/userdata_view_model.h"

/// @brief The trace files a capture module has produced, newest first.
class CaptureFileListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    /// @brief The roles the capture view binds to.
    enum Roles : int
    {
        kNameRole = Qt::UserRole + 1,  ///< The file's name.
        kSizeRole,                     ///< The file's size, formatted for display.
        kModifiedRole,                 ///< When the file was last written, formatted for display.
        kPathRole                      ///< The file's full path.
    };
    Q_ENUM(Roles)

    /// @brief Constructor.
    /// @param [in] parent The parent object.
    explicit CaptureFileListModel(QObject* parent = nullptr);

    /// @brief Destructor.
    ~CaptureFileListModel() override;

    /// @brief Re-reads the output directory.
    /// @param [in] directory The directory holding the module's trace files.
    /// @param [in] extension The extension of the files the module writes.
    void Refresh(const QString& directory, const QString& extension);

    /// @brief Gets the full path of the file in the given row.
    /// @param [in] row The file's row.
    /// @return The path, or empty when the row is out of range.
    [[nodiscard]] QString PathAt(int row) const;

    /// @brief Forgets the file in the given row, after it has been removed from disk.
    /// @param [in] row The file's row.
    void RemoveRow(int row);

    /// @brief QAbstractListModel::rowCount() override.
    /// @param [in] parent The parent index.
    /// @return The number of files.
    int rowCount(const QModelIndex& parent) const override;

    /// @brief QAbstractListModel::data() override.
    /// @param [in] index The index to query.
    /// @param [in] role The role to query.
    /// @return The data for the index and role.
    QVariant data(const QModelIndex& index, int role) const override;

    /// @brief QAbstractListModel::roleNames() override.
    /// @return The role names the capture view binds to.
    QHash<int, QByteArray> roleNames() const override;

private:
    /// @brief One trace file on disk.
    struct Entry
    {
        QString   name;      ///< The file's name.
        QString   path;      ///< The file's full path.
        QString   size;      ///< The file's size, formatted for display.
        QDateTime modified;  ///< When the file was last written.
    };

    std::vector<Entry> entries_;  ///< The files, newest first.
};

/// @brief The QML-facing view model shared by the capture modules.
///
/// TraceSourceViewModel reports through signals carrying C++ types, which QML cannot bind
/// to. This turns that into properties: it is the QML counterpart of the BaseUtilityView /
/// SplitClientView pair the Qt Widgets front-end shared between the capture modules.
class CaptureViewModel : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(QString statusText READ StatusText NOTIFY statusChanged)
    Q_PROPERTY(QColor statusColor READ StatusColor NOTIFY statusChanged)
    Q_PROPERTY(QString connectedProcessText READ ConnectedProcessText NOTIFY statusChanged)

    /// False while the module is not in a state where a capture can be asked for.
    Q_PROPERTY(bool captureEnabled READ IsCaptureEnabled NOTIFY statusChanged)
    Q_PROPERTY(QString disabledReason READ DisabledReason NOTIFY statusChanged)

    Q_PROPERTY(QString outputPath READ OutputPath WRITE SetOutputPath NOTIFY outputPathChanged)
    /// The path as the user wrote it, macros and all. Shown while the field is being edited.
    Q_PROPERTY(QString rawOutputPath READ RawOutputPath NOTIFY outputPathChanged)
    /// True while the output path does not name a directory traces can be written to.
    Q_PROPERTY(bool outputPathInvalid READ IsOutputPathInvalid NOTIFY outputPathChanged)

    Q_PROPERTY(bool progressVisible READ IsProgressVisible NOTIFY progressChanged)
    Q_PROPERTY(bool progressCancellable READ IsProgressCancellable NOTIFY progressChanged)
    Q_PROPERTY(qreal progress READ Progress NOTIFY progressChanged)
    /// The trace source reports progress 0 for the stages whose length it cannot know.
    Q_PROPERTY(bool progressIndeterminate READ IsProgressIndeterminate NOTIFY progressChanged)
    Q_PROPERTY(QString progressText READ ProgressText NOTIFY progressChanged)

    Q_PROPERTY(QAbstractItemModel* files READ Files CONSTANT)
    /// False for a module that captures on its own rather than on request.
    Q_PROPERTY(bool capturable READ IsCapturable CONSTANT)

public:
    /// @brief Constructor.
    /// @param [in] source The trace source view model this wraps.
    /// @param [in] userdata The module's userdata view model, which owns the output path.
    /// @param [in] request_capture Called when the view asks for a capture. Modules that
    ///        capture on their own, such as Crash Analysis, leave this empty.
    /// @param [in] parent The parent object.
    explicit CaptureViewModel(const std::shared_ptr<TraceSourceViewModel>& source,
                              const std::shared_ptr<UserdataViewModel>&    userdata,
                              std::function<void()>                        request_capture = {},
                              QObject*                                     parent          = nullptr);

    /// @brief Destructor.
    ~CaptureViewModel() override;

    /// @brief Gets the module's current status text.
    /// @return The status text.
    [[nodiscard]] QString StatusText() const;

    /// @brief Gets the colour the status should be drawn in.
    /// @return The status colour.
    [[nodiscard]] QColor StatusColor() const;

    /// @brief Gets the description of the connected process, if any.
    /// @return The connected process text.
    [[nodiscard]] QString ConnectedProcessText() const;

    /// @brief Returns whether a capture can currently be requested.
    /// @return true when the capture controls should be usable.
    [[nodiscard]] bool IsCaptureEnabled() const;

    /// @brief Gets why capture is unavailable.
    /// @return The reason, empty when capture is available.
    [[nodiscard]] QString DisabledReason() const;

    /// @brief Gets the directory the module writes its traces to.
    /// @return The output path.
    [[nodiscard]] QString OutputPath() const;

    /// @brief Sets the directory the module writes its traces to.
    /// @param [in] path The output path.
    void SetOutputPath(const QString& path);

    /// @brief Gets the output path with its macros left unexpanded.
    /// @return The raw output path.
    [[nodiscard]] QString RawOutputPath() const;

    /// @brief Returns whether the output path is unusable.
    /// @return true when traces cannot be written to the output path.
    [[nodiscard]] bool IsOutputPathInvalid() const;

    /// @brief Returns whether a capture is in progress.
    /// @return true while the progress bar should be shown.
    [[nodiscard]] bool IsProgressVisible() const;

    /// @brief Returns whether the operation in progress can be cancelled.
    /// @return true when it can be cancelled.
    [[nodiscard]] bool IsProgressCancellable() const;

    /// @brief Gets how far the operation in progress has got, in [0, 1].
    /// @return The progress.
    [[nodiscard]] qreal Progress() const;

    /// @brief Returns whether the progress of the current operation is unknown.
    /// @return true when the progress bar should run rather than fill.
    [[nodiscard]] bool IsProgressIndeterminate() const;

    /// @brief Gets the text shown alongside the progress bar.
    /// @return The progress text.
    [[nodiscard]] QString ProgressText() const;

    /// @brief Gets the trace files the module has produced.
    /// @return The file list model.
    [[nodiscard]] QAbstractItemModel* Files() const;

    /// @brief Opens the trace file in the given row with the module's tool.
    /// @param [in] row The file's row.
    Q_INVOKABLE void OpenFile(int row);

    /// @brief Deletes the trace file in the given row from disk.
    /// @param [in] row The file's row.
    Q_INVOKABLE void DeleteFile(int row);

    /// @brief Returns whether the module captures on request.
    /// @return true when the view should offer a capture control.
    [[nodiscard]] bool IsCapturable() const;

    /// @brief Asks the module to capture.
    Q_INVOKABLE void Capture();

    /// @brief Asks for the operation in progress to be abandoned.
    Q_INVOKABLE void Abort();

    /// @brief Re-reads the output directory.
    Q_INVOKABLE void RefreshFiles();

signals:
    /// @brief Emitted when the status, or whether capture is available, changes.
    void statusChanged();

    /// @brief Emitted when the output path changes.
    void outputPathChanged();

    /// @brief Emitted when a capture's progress changes.
    void progressChanged();

protected:
    /// @brief Gets the trace source view model this wraps.
    /// @return The trace source view model.
    [[nodiscard]] const std::shared_ptr<TraceSourceViewModel>& Source() const;

    /// @brief Gets the module's userdata view model.
    /// @return The userdata view model.
    [[nodiscard]] const std::shared_ptr<UserdataViewModel>& Userdata() const;

private:
    std::shared_ptr<TraceSourceViewModel>  source_;          ///< The trace source view model this wraps.
    std::shared_ptr<UserdataViewModel>     userdata_;        ///< The module's userdata view model.
    std::unique_ptr<CaptureFileListModel>  files_;           ///< The trace files the module has produced.
    std::function<void()>                  request_capture_;  ///< Asks the module to capture, empty when it captures on its own.

    QString      status_text_;                  ///< The module's current status text.
    QColor       status_color_;                 ///< The colour the status is drawn in.
    QString      connected_process_text_;       ///< The description of the connected process.
    QString      disabled_reason_;              ///< Why capture is unavailable, empty when it is available.
    QString      output_path_;                  ///< The directory the module writes its traces to, macros expanded.
    QString      raw_output_path_;              ///< The output path as the user wrote it.
    bool         capture_enabled_      = false; ///< true when the capture controls should be usable.
    bool         output_path_invalid_  = false; ///< true when traces cannot be written to the output path.
    ProgressInfo progress_             = {};    ///< The progress of the current capture.
};

#endif
