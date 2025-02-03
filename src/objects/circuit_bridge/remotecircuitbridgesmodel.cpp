/**
 * src/objects/circuit_bridge/remotecircuitbridgesmodel.cpp
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

#include "remotecircuitbridgesmodel.h"

#include "remotecircuitbridge.h"

#include <QColor>

RemoteCircuitBridgesModel::RemoteCircuitBridgesModel(ModeManager *mgr, QObject *parent)
    : AbstractSimulationObjectModel(mgr, RemoteCircuitBridge::Type, parent)
{

}

QVariant RemoteCircuitBridgesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case RemoteSession:
            return tr("Remote Session");
        case RemoteNode:
            return tr("Remote Node");
        default:
            break;
        }
    }

    return AbstractSimulationObjectModel::headerData(section, orientation, role);
}

int RemoteCircuitBridgesModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NColsExtra;
}

QVariant RemoteCircuitBridgesModel::data(const QModelIndex &idx, int role) const
{
    const RemoteCircuitBridge *bridge = static_cast<RemoteCircuitBridge *>(objectAt(idx.row()));
    if(!bridge)
        return QVariant();

    if(idx.column() == NameCol && role == Qt::DecorationRole)
    {
        // If bridge is remote, show red decoration
        if(bridge->isRemote())
            return QColor(Qt::red);

        return QVariant(); // Not decorated
    }
    else if(idx.column() == NodesCol)
    {
        const int count = bridge->getReferencingNodes(nullptr);
        const bool hasA = bridge->getNode(true);
        const bool hasB = bridge->getNode(false);
        bool wrongCount = false;
        if(bridge->isRemote() && (!hasA || hasB))
            wrongCount = true;
        else if(!bridge->isRemote() && (!hasA || !hasB))
            wrongCount = true;

        if(!wrongCount && count > 2)
            wrongCount = true;

        return nodesCountData(bridge, role,
                              count, wrongCount);
    }
    else if(idx.column() == RemoteSession && role == Qt::DisplayRole)
    {
        return bridge->remoteSessionName();
    }
    else if(idx.column() == RemoteNode && role == Qt::DisplayRole)
    {
        return bridge->peerNodeName();
    }

    return AbstractSimulationObjectModel::data(idx, role);
}
