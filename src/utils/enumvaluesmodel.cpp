/**
 * src/utils/enumvaluesmodel.cpp
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

#include "enumvaluesmodel.h"

EnumValuesModel::EnumValuesModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

int EnumValuesModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mValues.size();
}

QVariant EnumValuesModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mValues.size() || idx.column() > 0)
        return QVariant();

    const int value = valueAt(idx.row());

    if(role == Qt::DisplayRole)
        return mEnumDesc.name(value);
    else if(role == Qt::EditRole)
        return value;

    return QVariant();
}

int EnumValuesModel::valueAt(int row) const
{
    return mValues.value(row, -1);
}

int EnumValuesModel::rowForValue(int value) const
{
    return mValues.indexOf(value);
}

EnumDesc EnumValuesModel::enumDesc() const
{
    return mEnumDesc;
}

void EnumValuesModel::setEnumDescFull(const EnumDesc &newEnumDesc,
                                      bool skipMiddleValues)
{
    setEnumDescRange(newEnumDesc, skipMiddleValues,
                     newEnumDesc.minValue, newEnumDesc.maxValue);
}

void EnumValuesModel::setEnumDescRange(const EnumDesc &newEnumDesc,
                                       bool skipMiddleValues,
                                       int minVal, int maxVal)
{
    QVector<int> values;

    minVal = qMax(minVal, newEnumDesc.minValue);
    maxVal = qMin(maxVal, newEnumDesc.maxValue);

    // To skip middle values we increase by 2 at each iteration
    const int increment = skipMiddleValues ? 2 : 1;

    for(int i = minVal; i <= maxVal; i+= increment)
    {
        values.append(i);
    }

    setEnumDescFiltered(newEnumDesc, values);
}

void EnumValuesModel::setEnumDescFiltered(const EnumDesc &newEnumDesc,
                                          const QVector<int> &values)
{
    beginResetModel();
    mEnumDesc = newEnumDesc;
    mValues = values;
    endResetModel();
}
