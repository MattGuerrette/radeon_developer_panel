// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Definition for the model behind the output log view.

#ifndef RDP_SOURCE_FRONTEND_LOGGING_LOGGING_VIEW_MODEL_H_
#define RDP_SOURCE_FRONTEND_LOGGING_LOGGING_VIEW_MODEL_H_

#include <memory>

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <common/inc/api/dev_tools_logging.h>

#include <dd_common_api.h>

namespace rdp
{
    class LoggingModel;

    /// @brief Filters the log by source, process and UMD connection.
    class LoggingFilterProxyModel final : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        /// @brief The roles the output log binds to.
        enum Roles : int
        {
            kTimestampRole = Qt::UserRole + 1,  ///< When the message was logged.
            kLevelRole,                         ///< The severity, as a LogLevel.
            kLevelNameRole,                     ///< The severity's display name.
            kSourceRole,                        ///< What produced the message.
            kPidRole,                           ///< The process the message came from.
            kUmdIdRole,                         ///< The UMD connection the message came from.
            kMessageRole                        ///< The message itself.
        };
        Q_ENUM(Roles)

        /// @brief Constructor.
        /// @param [in] parent The parent object.
        explicit LoggingFilterProxyModel(QObject* parent = nullptr);

        /// @brief QSortFilterProxyModel::data() override, mapping the columns onto roles.
        /// @param [in] index The index to query.
        /// @param [in] role The role to query.
        /// @return The data for the index and role.
        QVariant data(const QModelIndex& index, int role) const override;

        /// @brief QSortFilterProxyModel::roleNames() override.
        /// @return The role names the output log binds to.
        QHash<int, QByteArray> roleNames() const override;

        /// @brief Sets the filters.
        /// @param [in] source The source to filter by, or empty for no filter.
        /// @param [in] pid The PID to filter by, or kLoggingInvalidPid for no filter.
        /// @param [in] umd_id The UMD Id to filter by, or kLoggingInvalidUmdId for no filter.
        void SetFilters(const QString& source, uint32_t pid, DDConnectionId umd_id);

    protected:
        /// @brief QSortFilterProxyModel::filterAcceptsRow() override.
        /// @param [in] source_row The row in the source model.
        /// @param [in] source_parent The parent of the row in the source model.
        /// @return true when the row passes every active filter.
        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    private:
        QString        source_;                         ///< The source to filter by.
        uint32_t       pid_    = kLoggingInvalidPid;    ///< The PID to filter by.
        DDConnectionId umd_id_ = kLoggingInvalidUmdId;  ///< The UMD Id to filter by.
    };

    /// @brief The model behind the output log view.
    ///
    /// Owns the filtering the Qt Widgets front-end did inside DebugLogWidget, so the QML
    /// view is only responsible for presenting it.
    class LoggingViewModel final : public QObject
    {
        Q_OBJECT
        QML_ANONYMOUS

        Q_PROPERTY(QAbstractItemModel* entries READ Entries CONSTANT)
        Q_PROPERTY(QAbstractItemModel* sourceFilters READ SourceFilters CONSTANT)
        Q_PROPERTY(QAbstractItemModel* pidFilters READ PidFilters CONSTANT)
        Q_PROPERTY(QAbstractItemModel* umdIdFilters READ UmdIdFilters CONSTANT)

        Q_PROPERTY(int sourceFilterIndex READ SourceFilterIndex WRITE SetSourceFilterIndex NOTIFY filtersChanged)
        Q_PROPERTY(int pidFilterIndex READ PidFilterIndex WRITE SetPidFilterIndex NOTIFY filtersChanged)
        Q_PROPERTY(int umdIdFilterIndex READ UmdIdFilterIndex WRITE SetUmdIdFilterIndex NOTIFY filtersChanged)
        /// The UMD connection only means anything once a single process has been picked.
        Q_PROPERTY(bool umdFilterEnabled READ IsUmdFilterEnabled NOTIFY filtersChanged)

        Q_PROPERTY(QString logFilePath READ LogFilePath NOTIFY logFilePathChanged)
        Q_PROPERTY(bool hasLogFile READ HasLogFile NOTIFY logFilePathChanged)

    public:
        /// @brief Constructor.
        /// @param [in] logging_model The model holding the logged messages.
        /// @param [in] parent The parent object.
        explicit LoggingViewModel(std::shared_ptr<LoggingModel> logging_model, QObject* parent = nullptr);

        /// @brief Destructor.
        ~LoggingViewModel() override;

        /// @brief Gets the filtered log.
        /// @return The log entries.
        [[nodiscard]] QAbstractItemModel* Entries() const;

        /// @brief Gets the list of sources that have logged something.
        /// @return The source filter model.
        [[nodiscard]] QAbstractItemModel* SourceFilters() const;

        /// @brief Gets the list of processes that have logged something.
        /// @return The PID filter model.
        [[nodiscard]] QAbstractItemModel* PidFilters() const;

        /// @brief Gets the list of UMD connections for the selected process.
        /// @return The UMD Id filter model.
        [[nodiscard]] QAbstractItemModel* UmdIdFilters() const;

        /// @brief Gets the selected row of the source filter list.
        /// @return The selected row.
        [[nodiscard]] int SourceFilterIndex() const;

        /// @brief Selects a source to filter by.
        /// @param [in] index The row of the source filter list.
        void SetSourceFilterIndex(int index);

        /// @brief Gets the selected row of the PID filter list.
        /// @return The selected row.
        [[nodiscard]] int PidFilterIndex() const;

        /// @brief Selects a process to filter by.
        /// @param [in] index The row of the PID filter list.
        void SetPidFilterIndex(int index);

        /// @brief Gets the selected row of the UMD Id filter list.
        /// @return The selected row.
        [[nodiscard]] int UmdIdFilterIndex() const;

        /// @brief Selects a UMD connection to filter by.
        /// @param [in] index The row of the UMD Id filter list.
        void SetUmdIdFilterIndex(int index);

        /// @brief Returns whether the UMD connection filter currently applies.
        /// @return true when a single process is selected.
        [[nodiscard]] bool IsUmdFilterEnabled() const;

        /// @brief Gets the path of the log file, or empty when none is open.
        /// @return The log file path.
        [[nodiscard]] QString LogFilePath() const;

        /// @brief Returns whether a log file is open.
        /// @return true when there is a log file.
        [[nodiscard]] bool HasLogFile() const;

        /// @brief Opens the folder holding the log file in the system file browser.
        Q_INVOKABLE void OpenLogFileLocation() const;

    signals:
        /// @brief Emitted when any of the filters, or which of them apply, changes.
        void filtersChanged();

        /// @brief Emitted when the log file path changes.
        void logFilePathChanged();

    private:
        /// @brief Pushes the selected filters into the proxy.
        void ApplyFilters();

        std::shared_ptr<LoggingModel>            logging_model_;  ///< The model holding the logged messages.
        std::unique_ptr<LoggingFilterProxyModel> entries_;        ///< The filtered log.

        int source_filter_index_ = 0;  ///< The selected row of the source filter list.
        int pid_filter_index_    = 0;  ///< The selected row of the PID filter list.
        int umd_id_filter_index_ = 0;  ///< The selected row of the UMD Id filter list.

        uint32_t last_selected_pid_ = kLoggingInvalidPid;  ///< The process the UMD filter list was last rebuilt for.
    };
}  // namespace rdp

#endif
