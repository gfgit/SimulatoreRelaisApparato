/**
 * src/objects/simple_activable/abstractsimpleactivableobject.cpp
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

#include "abstractsimpleactivableobject.h"

#include "../../circuits/nodes/simpleactivationnode.h"

AbstractSimpleActivableObject::AbstractSimpleActivableObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{

}

AbstractSimpleActivableObject::~AbstractSimpleActivableObject()
{
    const auto nodesCopy = mNodes;
    for(SimpleActivationNode *node : nodesCopy)
    {
        node->setObject(nullptr);
    }
}

int AbstractSimpleActivableObject::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int nodesCount = AbstractSimulationObject::getReferencingNodes(result);

    nodesCount += mNodes.size();

    if(result)
    {
        for(auto item : mNodes)
            result->append(item);
    }

    return nodesCount;
}

void AbstractSimpleActivableObject::addNode(SimpleActivationNode *node)
{
    Q_ASSERT_X(!mNodes.contains(node),
               "addNode", "already added");

    mNodes.append(node);

    emit nodesChanged(this);
}

void AbstractSimpleActivableObject::removeNode(SimpleActivationNode *node)
{
    Q_ASSERT_X(mNodes.contains(node),
               "removeNode", "not registered");
    Q_ASSERT_X(node->object() == this,
               "removeNode", "relay does not match");

    mNodes.removeOne(node);

    emit nodesChanged(this);
}

void AbstractSimpleActivableObject::onNodeStateChanged(SimpleActivationNode *node, bool val)
{
    const State oldState = state();

    if(val)
        mActiveNodesCount++;
    else
        mActiveNodesCount--;

    if(state() != oldState)
    {
        onStateChangedInternal();
        emit stateChanged(this);
    }
}

void AbstractSimpleActivableObject::onStateChangedInternal()
{

}
