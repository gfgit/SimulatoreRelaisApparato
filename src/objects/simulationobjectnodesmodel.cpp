/**
 * src/objects/simulationobjectnodesmodel.cpp
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

#include "simulationobjectnodesmodel.h"

#include "abstractsimulationobject.h"

#include "../circuits/circuitscene.h"
#include "../circuits/graphs/abstractnodegraphitem.h"
#include "../circuits/nodes/abstractcircuitnode.h"
#include "../circuits/edit/nodeeditfactory.h"

#include "../circuits/view/circuitlistmodel.h"

#include "../views/viewmanager.h"
#include "../views/modemanager.h"

SimulationObjectNodesModel::SimulationObjectNodesModel(ViewManager *viewMgr, QObject *parent)
    : QAbstractTableModel(parent)
    , mViewMgr(viewMgr)
{

}

QVariant SimulationObjectNodesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case NodeTypeCol:
            return tr("Type");
        case SceneNameCol:
            return tr("Circuit Scene");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int SimulationObjectNodesModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mItems.size();
}

int SimulationObjectNodesModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant SimulationObjectNodesModel::data(const QModelIndex &idx, int role) const
{
    if (role != Qt::DisplayRole || !idx.isValid() || idx.row() >= mItems.size() || idx.column() >= NCols)
        return QVariant();

    AbstractNodeGraphItem *item = mItems.at(idx.row());

    switch (idx.column())
    {
    case NodeTypeCol:
    {
        return mViewMgr->modeMgr()->circuitFactory()
                ->prettyName(item->getAbstractNode()->nodeType());
    }
    case SceneNameCol:
    {
        CircuitScene *s = item->circuitScene();
        if(s)
            return s->circuitSheetName();
        break;
    }
    }

    return QVariant();
}

AbstractSimulationObject *SimulationObjectNodesModel::getObject() const
{
    return mObject;
}

void SimulationObjectNodesModel::setObject(AbstractSimulationObject *newObject)
{
    if(mObject == newObject)
        return;

    if(mObject)
    {
        disconnect(mObject, &AbstractSimulationObject::nodesChanged,
                   this, &SimulationObjectNodesModel::refreshNodeList);
        disconnect(mObject, &AbstractSimulationObject::destroyed,
                   this, &SimulationObjectNodesModel::onObjectDestroyed);
    }

    mObject = newObject;

    if(mObject)
    {
        connect(mObject, &AbstractSimulationObject::nodesChanged,
                this, &SimulationObjectNodesModel::refreshNodeList);
        connect(mObject, &AbstractSimulationObject::destroyed,
                this, &SimulationObjectNodesModel::onObjectDestroyed);
    }

    refreshNodeList();
}

void SimulationObjectNodesModel::refreshNodeList()
{
    if(!mObject)
    {
        beginResetModel();
        mItems.clear();
        mItems.squeeze();
        endResetModel();
        return;
    }

    beginResetModel();

    mItems.clear();

    QVector<AbstractCircuitNode *> nodes;
    int count = mObject->getReferencingNodes(nullptr);
    nodes.reserve(count);
    mItems.reserve(count);

    mObject->getReferencingNodes(&nodes);

    NodeEditFactory *nodeFactory = mViewMgr->modeMgr()->circuitFactory();
    QStringList types = nodeFactory->getRegisteredTypes();

    // Sort according to type registering order
    std::stable_sort(nodes.begin(), nodes.end(),
                     [types](AbstractCircuitNode *a, AbstractCircuitNode *b) -> bool
    {
        if(a->nodeType() == b->nodeType())
            return false;

        return types.indexOf(a->nodeType()) < types.indexOf(b->nodeType());
    });

    auto circuitListModel = mViewMgr->modeMgr()->circuitList();

    for(AbstractCircuitNode *node : nodes)
    {
        AbstractNodeGraphItem *item = circuitListModel->getGraphForNode(node);
        if(!item)
            continue;

        mItems.append(item);
    }

    endResetModel();
}

void SimulationObjectNodesModel::onObjectDestroyed()
{
    mObject = nullptr;
    refreshNodeList();
}
