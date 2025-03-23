/**
 * src/panels/edit/lightrectlightsmodel.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "lightrectlightsmodel.h"

#include "../../objects/simple_activable/lightbulbobject.h"

LightRectLightsModel::LightRectLightsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant LightRectLightsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case LightCol:
            return tr("Light");
        case ColorCol:
            return tr("Color");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int LightRectLightsModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mItems.size();
}

int LightRectLightsModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant LightRectLightsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mItems.size() || idx.column() >= NCols)
        return QVariant();

    const LightEntry &entry = mItems.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
        switch (idx.column())
        {
        case LightCol:
            return entry.light->name();
        case ColorCol:
            return entry.color.name();
        default:
            break;
        }
        break;
    }
    case Qt::DecorationRole:
    {
        switch (idx.column())
        {
        case ColorCol:
            return entry.color;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
    return QVariant();
}

void LightRectLightsModel::setEntryAt(int row, const LightEntry &entry)
{
    if(row < 0 || row >= mItems.size())
        return;

    if(mItems.at(row) == entry)
        return;

    for(int i = 0; i < mItems.size(); i++)
    {
        if(i != row && mItems.at(i).light == entry.light)
            return;
    }

    mItems[row] = entry;
    setChanged();
    emit dataChanged(index(row, 0), index(row, NCols - 1));
}

void LightRectLightsModel::removeEntryAt(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    mItems.removeAt(row);
    setChanged();
    endRemoveRows();
}

void LightRectLightsModel::addEntryAt(int row, const LightEntry &entry)
{
    for(int i = 0; i < mItems.size(); i++)
    {
        if(mItems.at(i).light == entry.light)
            return;
    }

    row = qBound(0, row, mItems.size());
    beginInsertRows(QModelIndex(), row, row);
    mItems.insert(row, entry);
    setChanged();
    endInsertRows();
}

void LightRectLightsModel::moveRow(int sourceRow, bool up)
{
    int destinationChild = up ? sourceRow - 1 : sourceRow + 2;

    if (sourceRow < 0
            || sourceRow >= mItems.size()
            || destinationChild < 0
            || destinationChild > mItems.size()
            || sourceRow == destinationChild - 1)
    {
        return;
    }
    if (!beginMoveRows(QModelIndex(), sourceRow, sourceRow,
                       QModelIndex(), destinationChild))
        return;

    if(destinationChild > sourceRow)
        destinationChild--;

    const int fromRow = sourceRow;
    mItems.move(fromRow, destinationChild);
    endMoveRows();
}

QVector<LightRectLightsModel::LightEntry> LightRectLightsModel::items() const
{
    return mItems;
}

void LightRectLightsModel::setItems(const QVector<LightEntry> &newItems)
{
    beginResetModel();
    mItems = newItems;
    resetChanged();
    endResetModel();
}
