// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT

/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of the sortable table model behind the DataTable control.

#include "rdptablemodel.h"

#include <algorithm>
#include <numeric>

RdpTableModel::RdpTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    collator_.setCaseSensitivity(Qt::CaseInsensitive);
    collator_.setNumericMode(true);
}

QVariantList RdpTableModel::Rows() const
{
    return rows_;
}

void RdpTableModel::SetRows(const QVariantList& rows)
{
    if (rows_ == rows)
    {
        return;
    }

    rows_ = rows;
    RebuildOrder();

    emit rowsChanged();
}

QStringList RdpTableModel::Headers() const
{
    return headers_;
}

void RdpTableModel::SetHeaders(const QStringList& headers)
{
    if (headers_ == headers)
    {
        return;
    }

    // The column count is derived from the headers, so this is a structural change,
    // and a sort key that fell off the end has to go with it.
    const bool sort_key_gone = sort_column_ >= headers.size();

    beginResetModel();
    headers_ = headers;
    if (sort_key_gone)
    {
        sort_column_ = -1;
    }
    RebuildOrderLocked();
    endResetModel();

    if (sort_key_gone)
    {
        emit sortChanged();
    }

    emit headersChanged();
}

int RdpTableModel::PlaceholderRowCount() const
{
    return placeholder_row_count_;
}

void RdpTableModel::SetPlaceholderRowCount(const int count)
{
    if (placeholder_row_count_ == count)
    {
        return;
    }

    // Only visible while there is no real data to show.
    const bool affects_row_count = rows_.isEmpty();
    if (affects_row_count)
    {
        beginResetModel();
    }

    placeholder_row_count_ = count;

    if (affects_row_count)
    {
        endResetModel();
    }

    emit placeholderRowCountChanged();
}

int RdpTableModel::SortColumn() const
{
    return sort_column_;
}

Qt::SortOrder RdpTableModel::SortOrder() const
{
    return sort_order_;
}

void RdpTableModel::ToggleSort(const int column)
{
    if (column < 0 || column >= headers_.size())
    {
        return;
    }

    const Qt::SortOrder order = (column == sort_column_ && sort_order_ == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;

    sort(column, order);
}

void RdpTableModel::sort(const int column, const Qt::SortOrder order)
{
    if (column < -1 || column >= headers_.size())
    {
        return;
    }

    if (column == sort_column_ && order == sort_order_)
    {
        return;
    }

    sort_column_ = column;
    sort_order_  = order;
    RebuildOrder();

    emit sortChanged();
}

void RdpTableModel::RebuildOrder()
{
    beginResetModel();
    RebuildOrderLocked();
    endResetModel();
}

void RdpTableModel::RebuildOrderLocked()
{
    order_.resize(rows_.size());
    std::iota(order_.begin(), order_.end(), 0);

    if (sort_column_ >= 0 && sort_column_ < headers_.size())
    {
        // Stable, so rows that compare equal keep the order they were supplied in rather
        // than shuffling every time the direction is flipped.
        std::stable_sort(order_.begin(), order_.end(), [this](const int lhs, const int rhs) {
            const QVariant a = CellAt(lhs, sort_column_);
            const QVariant b = CellAt(rhs, sort_column_);
            return sort_order_ == Qt::AscendingOrder ? LessThan(a, b) : LessThan(b, a);
        });
    }
}

QVariant RdpTableModel::CellAt(const int source_row, const int column) const
{
    if (source_row < 0 || source_row >= rows_.size())
    {
        return {};
    }

    const QVariantList row = rows_.at(source_row).toList();
    if (column < 0 || column >= row.size())
    {
        return {};
    }

    return row.at(column);
}

bool RdpTableModel::LessThan(const QVariant& lhs, const QVariant& rhs) const
{
    // Cells arrive as whatever QML put in the row array, so the numeric columns RDP tables
    // are full of ("4096", "12.5") have to be compared as numbers before falling back to
    // text; otherwise "9" would outrank "10".
    bool         lhs_numeric = false;
    bool         rhs_numeric = false;
    const double lhs_number  = lhs.toDouble(&lhs_numeric);
    const double rhs_number  = rhs.toDouble(&rhs_numeric);
    if (lhs_numeric && rhs_numeric)
    {
        return lhs_number < rhs_number;
    }

    return collator_.compare(lhs.toString(), rhs.toString()) < 0;
}

int RdpTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    // Empty tables still draw striped rows, the way RDP's do before a capture.
    return rows_.isEmpty() ? placeholder_row_count_ : static_cast<int>(order_.size());
}

int RdpTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return static_cast<int>(headers_.size());
}

QVariant RdpTableModel::data(const QModelIndex& index, const int role) const
{
    if (role != Qt::DisplayRole || !index.isValid())
    {
        return {};
    }

    if (index.row() >= order_.size())
    {
        // A placeholder row: the table is drawing stripes with nothing in them.
        return QString();
    }

    return CellAt(order_.at(index.row()), index.column());
}

QVariant RdpTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return {};
    }

    if (section < 0 || section >= headers_.size())
    {
        return {};
    }

    return headers_.at(section);
}
