/**
 * src/objects/acei_lever/model/leverpositionmodel.cpp
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

LeverPositionModel::LeverPositionModel(const LeverPositionDesc &desc, QObject *parent)
    : QAbstractListModel(parent)
    , mPositionDesc(desc)
{
    mPositionMin = 0;
    mPositionMax = mPositionDesc.maxPosition();
}

int LeverPositionModel::rowCount(const QModelIndex &p) const
{
    if(p.isValid())
        return 0;

    // Calculate exact position count (skipping middle positions)
    int count = (mPositionMax - mPositionMin) + 1;

    return (((count - 1) / 2) + 1);
}

QVariant LeverPositionModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() ||
            idx.column() > 0 ||
            idx.row() >= mPositionDesc.exactPositionsCount())
        return QVariant();

    const int position = positionAt(idx.row());
    if(position > mPositionMax)
        return QVariant();

    if(role == Qt::DisplayRole)
        return mPositionDesc.nameFor(position);
    else if(role == Qt::EditRole)
        return position;

    return QVariant();
}

void LeverPositionModel::setPositionRange(int min, int max)
{
    beginResetModel();
    mPositionMin = min;
    mPositionMax = max;
    endResetModel();
}

int LeverPositionModel::positionAt(int row) const
{
    // Count middle positions in between
    return mPositionMin + row * 2;
}

int LeverPositionModel::rowForPosition(int position) const
{
    if(position < mPositionMin || position > mPositionMax)
        return -1;

    // We skip middle positions
    return (position - mPositionMin) / 2;
}
