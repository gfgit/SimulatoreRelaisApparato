/**
 * src/objects/lever/model/levercontactconditionsmodel.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
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

#include "levercontactconditionsmodel.h"

LeverContactConditionsModel::LeverContactConditionsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant LeverContactConditionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if(role == Qt::DisplayRole)
        {
            switch (section)
            {
            case TypeCol:
                return tr("Type");
            case FromCol:
                return tr("From");
            case ToCol:
                return tr("To");
            case SpecialCol:
                return tr("Special");
            default:
                break;
            }
        }
        else if(role == Qt::ToolTipRole)
        {
            switch (section)
            {
            case TypeCol:
                return tr("Contact Type");
            case FromCol:
                return tr("From position");
            case ToCol:
                return tr("To position");
            case SpecialCol:
                return tr("Special contacts briefly connect both sides.\n"
                          "So output always has current during lever switch.");
            default:
                break;
            }
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}


int LeverContactConditionsModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mConditions.size();
}

int LeverContactConditionsModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant LeverContactConditionsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mConditions.size() || idx.column() >= NCols)
        return QVariant();

    const LeverPositionCondition& item = mConditions.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case TypeCol:
            return GenericLeverUtils::getTypeName(item.type);
        case FromCol:
            return mPositionDesc.name(item.positionFrom);
        case ToCol:
            if(item.type != LeverPositionConditionType::FromTo)
                return QString();
            return mPositionDesc.name(item.positionTo);
        default:
            break;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TypeCol:
            return int(item.type);
        case FromCol:
            return item.positionFrom;
        case ToCol:
            return item.positionTo;
        case SpecialCol:
            return item.specialContact;
        default:
            break;
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case SpecialCol:
            return item.specialContact ?
                        Qt::CheckState::Checked :
                        Qt::CheckState::Unchecked;
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

bool LeverContactConditionsModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid() || idx.row() >= mConditions.size() || idx.column() >= NCols)
        return false;

    LeverPositionCondition& item = mConditions[idx.row()];

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TypeCol:
        {
            if(value.toInt() == int(LeverPositionConditionType::FromTo))
            {
                item.type = LeverPositionConditionType::FromTo;
                item.positionTo = item.positionFrom + 2; // Pass Middle, go next position

                if(mCanWarpAroundZero && item.positionTo > mPositionDesc.maxValue)
                    item.positionTo = item.positionTo - mPositionDesc.maxValue; // Warp
            }
            else
            {
                item.type = LeverPositionConditionType::Exact;
                item.positionTo = item.positionFrom;
            }
            break;
        }
        case FromCol:
        {
            item.positionFrom = value.toInt();
            break;
        }
        case ToCol:
        {
            if(item.type != LeverPositionConditionType::FromTo)
                return false;

            item.positionTo = value.toInt();
            break;
        }
        default:
            return false;
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case SpecialCol:
        {
            Qt::CheckState cs = value.value<Qt::CheckState>();
            item.specialContact = cs == Qt::CheckState::Checked;
            break;
        }
        default:
            return false;
        }
        break;
    }
    default:
        return false;
    }

    if(item.type == LeverPositionConditionType::FromTo
            && !mCanWarpAroundZero)
    {
        // Pass Middle, go next position
        item.positionTo = qMax(item.positionFrom + 2,
                               item.positionTo);
    }

    item.positionFrom = qBound(mPositionMin,
                               item.positionFrom,
                               mPositionMax);

    const int minToPos = mCanWarpAroundZero ? mPositionMin : item.positionFrom;
    item.positionTo = qBound(minToPos,
                             item.positionTo,
                             mPositionMax);

    // Reset to Exact if positions are equal
    if(item.positionFrom == item.positionTo)
        item.type = LeverPositionConditionType::Exact;

    // Check if it warps
    item.warpsAroundZero = (item.positionFrom > item.positionTo);

    emit changed();

    emit dataChanged(idx.siblingAtColumn(TypeCol),
                     idx.siblingAtColumn(ToCol));
    return true;
}

Qt::ItemFlags LeverContactConditionsModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.row() >= mConditions.size() || idx.column() >= NCols)
        return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    const LeverPositionCondition& item = mConditions.at(idx.row());
    if(idx.column() != ToCol || item.type == LeverPositionConditionType::FromTo)
        f.setFlag(Qt::ItemIsEditable);

    if(idx.column() == SpecialCol)
        f.setFlag(Qt::ItemIsUserCheckable);

    return f;
}

LeverPositionConditionSet LeverContactConditionsModel::conditions() const
{
    return mConditions;
}

void LeverContactConditionsModel::setConditions(const EnumDesc& desc,
                                                const LeverPositionConditionSet &newConditions,
                                                bool canWarp)
{
    beginResetModel();

    mPositionDesc = desc;
    mConditions = newConditions;
    mCanWarpAroundZero = canWarp;

    // Resets range
    setPositionRange(mPositionDesc.minValue, mPositionDesc.maxValue);

    endResetModel();
}

void LeverContactConditionsModel::instertConditionAt(int row)
{
    LeverPositionCondition item;
    item.type = LeverPositionConditionType::Exact;
    item.positionFrom = 0;

    beginInsertRows(QModelIndex(), row, row);
    mConditions.insert(row, item);
    endInsertRows();

    emit changed();
}

void LeverContactConditionsModel::removeConditionAt(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    mConditions.removeAt(row);
    endRemoveRows();

    emit changed();
}

void LeverContactConditionsModel::setPositionRange(int min, int max)
{
    if(mCanWarpAroundZero)
    {
        // Continuous rotation levers always use full range
        mPositionMin = mPositionDesc.minValue;
        mPositionMax = mPositionDesc.maxValue;
        return;
    }

    min = qBound(mPositionDesc.minValue,
                 min,
                 mPositionDesc.maxValue);
    max = qBound(min,
                 max,
                 mPositionDesc.maxValue);

    mPositionMin = min;
    mPositionMax = max;

    for(LeverPositionCondition& item : mConditions)
    {
        item.positionFrom = qBound(mPositionMin,
                                   item.positionFrom,
                                   mPositionMax);

        item.positionTo = qBound(item.positionFrom,
                                 item.positionTo,
                                 mPositionMax);

        // Reset to Exact if positions are equal
        if(item.positionFrom == item.positionTo)
            item.type = LeverPositionConditionType::Exact;
    }

    emit changed();
}

std::pair<int, int> LeverContactConditionsModel::positionRangeFor(const QModelIndex &idx) const
{
    if(!idx.isValid() || idx.row() >= mConditions.size())
        return {-1, -1};

    const LeverPositionCondition& item = mConditions.at(idx.row());

    if(idx.column() == FromCol)
    {
        // TODO: sort items and return prev/next
        return {mPositionMin, mPositionMax};
    }

    if(idx.column() == ToCol)
    {
        // TODO: sort items and return prev/next
        if(mCanWarpAroundZero)
            return {mPositionMin, mPositionMax};
        return {item.positionFrom + 2, mPositionMax};
    }

    return {-1, -1};
}

const EnumDesc &LeverContactConditionsModel::positionDesc() const
{
    return mPositionDesc;
}
