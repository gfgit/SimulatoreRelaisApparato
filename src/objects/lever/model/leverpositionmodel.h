/**
 * src/objects/lever/model/leverpositionmodel.h
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

#ifndef LEVERPOSITIONMODEL_H
#define LEVERPOSITIONMODEL_H

#include <QAbstractListModel>

class LeverPositionDesc;

class LeverPositionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    LeverPositionModel(const LeverPositionDesc& desc, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    void setPositionRange(int min, int max);

    int positionAt(int row) const;
    int rowForPosition(int position) const;

private:
    const LeverPositionDesc &mPositionDesc;

    int mPositionMin = 0;
    int mPositionMax = 0;
};

#endif // LEVERPOSITIONMODEL_H
