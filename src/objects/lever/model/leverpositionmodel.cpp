/**
 * src/objects/lever/model/leverpositionmodel.cpp
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

#include "leverpositionmodel.h"

#include "../../../utils/genericleverutils.h"

EnumValuesModel::EnumValuesModel(const EnumDesc &desc, QObject *parent)
    : QAbstractListModel(parent)
    , mEnumDesc(desc)
{
    mMinValue = mEnumDesc.minValue;
    mMaxValue = mEnumDesc.maxValue;
}

int EnumValuesModel::rowCount(const QModelIndex &p) const
{
    if(p.isValid())
        return 0;

    int count = (mMaxValue - mMinValue) + 1;

    if(mSkipMiddleValues)
    {
        // Calculate exact position count (skipping middle positions)
        return (((count - 1) / 2) + 1);
    }

    return count;
}

QVariant EnumValuesModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() ||
            idx.column() > 0 ||
            idx.row() >= rowCount())
        return QVariant();

    const int value = valueAt(idx.row());
    if(value > mMaxValue)
        return QVariant();

    if(role == Qt::DisplayRole)
        return mEnumDesc.name(value);
    else if(role == Qt::EditRole)
        return value;

    return QVariant();
}

void EnumValuesModel::setValueRange(int min, int max)
{
    beginResetModel();
    mMinValue = min;
    mMaxValue = max;
    endResetModel();
}

int EnumValuesModel::valueAt(int row) const
{
    if(mSkipMiddleValues)
    {
        // Count middle positions in between
        return mMinValue + row * 2;
    }

    return mMinValue + row;
}

int EnumValuesModel::rowForValue(int position) const
{
    if(position < mMinValue || position > mMaxValue)
        return -1;

    if(mSkipMiddleValues)
    {
        // We skip middle positions
        return (position - mMinValue) / 2;
    }

    return position - mMinValue;
}

bool EnumValuesModel::skipMiddleValues() const
{
    return mSkipMiddleValues;
}

void EnumValuesModel::setSkipMiddleValues(bool newSkipMiddleValues)
{
    beginResetModel();
    mSkipMiddleValues = newSkipMiddleValues;
    endResetModel();
}
