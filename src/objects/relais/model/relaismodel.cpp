/**
 * src/objects/relais/model/relaismodel.cpp
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

#include "relaismodel.h"

#include "abstractrelais.h"

#include "../../../views/modemanager.h"

#include "../../../circuits/graphs/circuitcolors.h"

#include <QColor>

#include <QJsonObject>
#include <QJsonArray>

#include <QFont>

RelaisModel::RelaisModel(ModeManager *mgr, QObject *parent)
    : AbstractSimulationObjectModel(mgr, AbstractRelais::Type, parent)
{
}

QVariant RelaisModel::headerData(int section, Qt::Orientation orientation, int role) const
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

int RelaisModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NColsExtra;
}

QVariant RelaisModel::data(const QModelIndex &idx, int role) const
{
    const AbstractRelais *relay = static_cast<AbstractRelais *>(objectAt(idx.row()));
    if(!relay)
        return QVariant();

    if(idx.column() == NameCol && role == Qt::DecorationRole)
    {
        // Show a little colored square based on relay state
        QColor color = Qt::black;
        switch (relay->state())
        {
        case AbstractRelais::State::Up:
            color = CircuitColors::Closed; // Red
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color = CircuitColors::Open; // Light blue
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }

        if(modeMgr()->mode() == FileMode::Editing)
        {
            // During editing we highlight special relays
            switch (relay->relaisType())
            {
            case AbstractRelais::RelaisType::Encoder:
            case AbstractRelais::RelaisType::CodeRepeater:
                color = Qt::darkCyan;
                break;
            case AbstractRelais::RelaisType::Blinker:
                color = Qt::darkGreen;
                break;
            case AbstractRelais::RelaisType::Timer:
                color = Qt::darkMagenta;
                break;
            default:
            {
                // For other types, show default state
                if(relay->normallyUp())
                    color = CircuitColors::Closed; // Red
                else
                    color = Qt::black;
                break;
            }
            }
        }

        return color;
    }
    else if(idx.column() == NameCol && role == Qt::ToolTipRole)
    {
        return tr("Relay: <b>%1</b><br>"
                  "Type: %2<br>"
                  "Default state: %3")
                .arg(relay->name(),
                     AbstractRelais::getRelaisTypeName(relay->relaisType()),
                     relay->normallyUp() ? tr("Up") : tr("Down"));
    }
    else if(idx.column() == PowerNodes)
    {
        const int count = relay->getPowerNodesCount();
        return nodesCountData(relay, role,
                              count, (count == 0 || count > 2),
                              tr("Relay <b>%1</b> is powered by <b>%2</b> nodes.")
                              .arg(relay->name()).arg(count));
    }
    else if(idx.column() == ContactNodes)
    {
        const int count = relay->getContactNodesCount();
        return nodesCountData(relay, role,
                              count, count == 0,
                              tr("Relay <b>%1</b> contacts are used in <b>%2</b> nodes.")
                              .arg(relay->name()).arg(count));
    }

    QModelIndex baseIdx = idx;
    if(idx.column() == TotalNodes)
        baseIdx = baseIdx.siblingAtColumn(NodesCol);

    return AbstractSimulationObjectModel::data(baseIdx, role);
}
