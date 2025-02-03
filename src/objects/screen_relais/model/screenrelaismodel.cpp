/**
 * src/objects/screen_relais/model/screenrelaismodel.cpp
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

#include "screenrelaismodel.h"

#include "screenrelais.h"

#include "../../../views/modemanager.h"

#include <QColor>

#include <QJsonObject>
#include <QJsonArray>

#include <QFont>

ScreenRelaisModel::ScreenRelaisModel(ModeManager *mgr, QObject *parent)
    : AbstractSimulationObjectModel(mgr, ScreenRelais::Type, parent)
{
}

QVariant ScreenRelaisModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case PowerNodes:
            return tr("Power");
        case ContactNodes:
            return tr("Contacts");
        case TotalNodes:
            return tr("Tot. Nodes");
        default:
            break;
        }
    }

    return AbstractSimulationObjectModel::headerData(section, orientation, role);
}

int ScreenRelaisModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NColsExtra;
}

QVariant ScreenRelaisModel::data(const QModelIndex &idx, int role) const
{
    const ScreenRelais *relay = static_cast<ScreenRelais *>(objectAt(idx.row()));
    if(!relay)
        return QVariant();

    if(idx.column() == PowerNodes)
    {
        const int count = relay->hasPowerNode() ? 1 : 0;
        return nodesCountData(relay, role,
                              count, count == 0,
                              tr("Screen Relay <b>%1</b> is powered by <b>%2</b> node.")
                              .arg(relay->name()).arg(count));
    }
    else if(idx.column() == ContactNodes)
    {
        const int count = relay->getContactNodesCount();
        return nodesCountData(relay, role,
                              count, count == 0,
                              tr("Screen Relay <b>%1</b> contacts are used in <b>%2</b> nodes.")
                              .arg(relay->name()).arg(count));
    }

    QModelIndex baseIdx = idx;
    if(idx.column() == TotalNodes)
        baseIdx = baseIdx.siblingAtColumn(NodesCol);

    return AbstractSimulationObjectModel::data(baseIdx, role);
}
