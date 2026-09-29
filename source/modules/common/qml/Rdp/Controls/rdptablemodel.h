// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Declaration of the sortable table model behind the DataTable control.

#ifndef RDP_SOURCE_FRONTEND_QML_CONTROLS_RDPTABLEMODEL_H_
#define RDP_SOURCE_FRONTEND_QML_CONTROLS_RDPTABLEMODEL_H_

#include <QAbstractTableModel>
#include <QCollator>
#include <QStringList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

/// @brief Adapts DataTable's "list of row arrays" data onto QAbstractItemModel.
///
/// TableView needs a QAbstractItemModel, and Qt.labs.qmlmodels' TableModel is not usable
/// here: it needs TableModelColumn children declared up front, while DataTable's columns
/// are supplied per call site.
///
/// Sorting is done in the model rather than behind a QSortFilterProxyModel: the rows are
/// already held here as a flat list, so an index permutation is both cheaper and keeps
/// DataTable talking to a single model object.
class RdpTableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList rows READ Rows WRITE SetRows NOTIFY rowsChanged)
    Q_PROPERTY(QStringList headers READ Headers WRITE SetHeaders NOTIFY headersChanged)
    Q_PROPERTY(int placeholderRowCount READ PlaceholderRowCount WRITE SetPlaceholderRowCount NOTIFY placeholderRowCountChanged)
    /// -1 while unsorted, which is the state every table starts in.
    Q_PROPERTY(int sortColumn READ SortColumn NOTIFY sortChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ SortOrder NOTIFY sortChanged)

public:
    /// @brief Constructor.
    /// @param [in] parent The parent object.
    explicit RdpTableModel(QObject* parent = nullptr);

    /// @brief Gets the rows the table is showing.
    /// @return The rows, each a list of cell values.
    [[nodiscard]] QVariantList Rows() const;

    /// @brief Sets the rows the table shows.
    /// @param [in] rows The rows, each a list of cell values.
    void SetRows(const QVariantList& rows);

    /// @brief Gets the column headers.
    /// @return The header titles.
    [[nodiscard]] QStringList Headers() const;

    /// @brief Sets the column headers. The column count is derived from these.
    /// @param [in] headers The header titles.
    void SetHeaders(const QStringList& headers);

    /// @brief Gets the number of empty rows drawn when there is no data.
    /// @return The placeholder row count.
    [[nodiscard]] int PlaceholderRowCount() const;

    /// @brief Sets the number of empty rows drawn when there is no data.
    /// @param [in] count The placeholder row count.
    void SetPlaceholderRowCount(int count);

    /// @brief Gets the column the rows are sorted on, or -1 when unsorted.
    /// @return The sort column.
    [[nodiscard]] int SortColumn() const;

    /// @brief Gets the direction the rows are sorted in.
    /// @return The sort order.
    [[nodiscard]] Qt::SortOrder SortOrder() const;

    /// @brief Sorts ascending on a newly picked column, and flips the direction when the
    /// column already drives the sort - the cycle QHeaderView gives RDP's tables.
    /// @param [in] column The column that was clicked.
    Q_INVOKABLE void ToggleSort(int column);

    /// @brief QAbstractTableModel::sort() override. Pass -1 to restore the natural order.
    /// @param [in] column The column to sort on.
    /// @param [in] order The direction to sort in.
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    /// @brief QAbstractTableModel::rowCount() override.
    /// @param [in] parent The parent index.
    /// @return The number of rows.
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief QAbstractTableModel::columnCount() override.
    /// @param [in] parent The parent index.
    /// @return The number of columns.
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief QAbstractTableModel::data() override.
    /// @param [in] index The index to query.
    /// @param [in] role The role to query.
    /// @return The cell value.
    QVariant data(const QModelIndex& index, int role) const override;

    /// @brief QAbstractTableModel::headerData() override.
    /// @param [in] section The column.
    /// @param [in] orientation The header orientation.
    /// @param [in] role The role to query.
    /// @return The header title.
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
    /// @brief Emitted when the rows change.
    void rowsChanged();

    /// @brief Emitted when the headers change.
    void headersChanged();

    /// @brief Emitted when the placeholder row count changes.
    void placeholderRowCountChanged();

    /// @brief Emitted when the sort column or direction changes.
    void sortChanged();

private:
    /// @brief Recomputes the row order from the current rows and sort key.
    ///
    /// Wrapped in a model reset, because every visible row can change identity at once.
    void RebuildOrder();

    /// @brief RebuildOrder() for callers that already hold a model reset open.
    void RebuildOrderLocked();

    /// @brief Gets the value of one cell of the unsorted row list.
    /// @param [in] source_row The row's index in the supplied row list.
    /// @param [in] column The column.
    /// @return The cell value, or an invalid variant when out of range.
    [[nodiscard]] QVariant CellAt(int source_row, int column) const;

    /// @brief Orders two cell values, numerically when both parse as numbers.
    /// @param [in] lhs The left value.
    /// @param [in] rhs The right value.
    /// @return true when lhs sorts before rhs.
    [[nodiscard]] bool LessThan(const QVariant& lhs, const QVariant& rhs) const;

    QVariantList rows_;                     ///< The rows as supplied, in their natural order.
    QStringList  headers_;                  ///< The column headers.
    int          placeholder_row_count_ = 0;  ///< Empty rows drawn when there is no data.

    QList<int>    order_;                              ///< View row -> index into rows_. Always as long as rows_.
    int           sort_column_ = -1;                   ///< The column driving the sort, -1 when unsorted.
    Qt::SortOrder sort_order_  = Qt::AscendingOrder;   ///< The direction the sort runs in.
    QCollator     collator_;                           ///< Numeric-mode collator, so "Frame 2" precedes "Frame 10".
};

#endif
