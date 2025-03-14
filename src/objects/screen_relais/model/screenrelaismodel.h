/**
 * src/objects/screen_relais/model/screenrelaismodel.h
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

#ifndef SCREEN_RELAIS_MODEL_H
#define SCREEN_RELAIS_MODEL_H

#include "../../abstractsimulationobjectmodel.h"

class ScreenRelaisModel : public AbstractSimulationObjectModel
{
    Q_OBJECT

public:
    enum ExtraColumns
    {
        PowerNodes = Columns::NodesCol,
        ContactNodes = Columns::NCols,
        TotalNodes,
        NColsExtra
    };

    ScreenRelaisModel(ModeManager *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    // Custom relay specific data:
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
};

#endif // SCREEN_RELAIS_MODEL_H
