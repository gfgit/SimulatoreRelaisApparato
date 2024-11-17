/**
 * src/circuits/nodes/simpleactivationnode.cpp
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

#include "simpleactivationnode.h"

#include "../../objects/simple_activable/abstractsimpleactivableobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

SimpleActivationNode::SimpleActivationNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));
}

QVector<CableItem> SimpleActivationNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 || !mContacts.at(0).cable)
        return {};

    // Close the circuit
    CableItem dest;
    dest.cable.cable = mContacts.at(0).cable;
    dest.cable.side = mContacts.at(0).cableSide;
    dest.nodeContact = 0;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void SimpleActivationNode::addCircuit(ElectricCircuit *circuit)
{
    const bool wasActive = hasCircuits();

    AbstractCircuitNode::addCircuit(circuit);

    const bool isActive = hasCircuits();

    if(!wasActive && isActive && mObject)
    {
        mObject->onNodeStateChanged(this, true);
    }
}

void SimpleActivationNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const bool wasActive = hasCircuits();

    AbstractCircuitNode::removeCircuit(circuit, items);

    const bool isActive = hasCircuits();

    if(wasActive && !isActive && mObject)
    {
        mObject->onNodeStateChanged(this, false);
    }
}

bool SimpleActivationNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    auto model = modeMgr()->modelForType(allowedObjectType());
    if(model)
    {
        const QString objName = obj.value("object").toString();
        AbstractSimulationObject *activationObj = model->getObjectByName(objName);
        setObject(static_cast<AbstractSimpleActivableObject *>(activationObj));
    }
    else
        setObject(nullptr);

    return true;
}

void SimpleActivationNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["object"] = mObject ? mObject->name() : QString();
}

AbstractSimpleActivableObject *SimpleActivationNode::object() const
{
    return mObject;
}

void SimpleActivationNode::setObject(AbstractSimpleActivableObject *newObject)
{
    if(mObject == newObject)
        return;

    if(newObject->getType() != allowedObjectType())
        return;

    if(mObject && hasCircuits())
    {
        mObject->onNodeStateChanged(this, false);
    }

    mObject = newObject;

    if(mObject && hasCircuits())
    {
        mObject->onNodeStateChanged(this, true);
    }

    emit objectChanged(mObject);
    modeMgr()->setFileEdited();
}
