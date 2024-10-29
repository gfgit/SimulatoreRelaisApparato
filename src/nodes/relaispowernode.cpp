/**
 * src/nodes/relaispowernode.cpp
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

#include "relaispowernode.h"

#include "../objects/abstractrelais.h"
#include "../objects/relaismodel.h"

#include <QJsonObject>

RelaisPowerNode::RelaisPowerNode(QObject *parent)
    : AbstractCircuitNode{true, parent}
{
    // 1 side
    mContacts.append(NodeContact("41", "42"));
}

RelaisPowerNode::~RelaisPowerNode()
{
    setRelais(nullptr);
}

QVector<CableItem> RelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0)
        return {};

    // Close the circuit
    // TODO: polarized relays?
    CableItem dest;
    dest.cable.cable = mContacts.at(0).cable;
    dest.cable.side = mContacts.at(0).cableSide;
    dest.nodeContact = 0;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void RelaisPowerNode::addCircuit(ElectricCircuit *circuit)
{
    const CircuitList &closedCircuits = getCircuits(CircuitType::Closed);
    bool wasEmpty = closedCircuits.isEmpty();

    AbstractCircuitNode::addCircuit(circuit);

    if(mRelais && wasEmpty && !closedCircuits.isEmpty())
    {
        mRelais->powerNodeActivated(this);
    }
}

void RelaisPowerNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const CircuitList &closedCircuits = getCircuits(CircuitType::Closed);
    bool hadCircuit = !closedCircuits.isEmpty();

    AbstractCircuitNode::removeCircuit(circuit, items);

    if(mRelais && hadCircuit && closedCircuits.isEmpty())
    {
        mRelais->powerNodeDeactivated(this);
    }
}

bool RelaisPowerNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString relaisName = obj.value("relais").toString();

    setRelais(relaisModel()->getRelay(relaisName));

    return true;
}

void RelaisPowerNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();
}

QString RelaisPowerNode::nodeType() const
{
    return NodeType;
}

AbstractRelais *RelaisPowerNode::relais() const
{
    return mRelais;
}

void RelaisPowerNode::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;

    if(mRelais)
        mRelais->removePowerNode(this);
    mRelais = newRelais;
    if(mRelais)
        mRelais->addPowerNode(this);

    emit relayChanged(mRelais);
}

RelaisModel *RelaisPowerNode::relaisModel() const
{
    return mRelaisModel;
}

void RelaisPowerNode::setRelaisModel(RelaisModel *newRelaisModel)
{
    mRelaisModel = newRelaisModel;
}
