/**
 * src/circuits/nodes/diodecircuitnode.cpp
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

#include "diodecircuitnode.h"

#include "../electriccircuit.h"

DiodeCircuitNode::DiodeCircuitNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, false, parent}
{
    // 2 sides
    mContacts.append(NodeContact()); // Anode
    mContacts.append(NodeContact()); // Cathode
}

AbstractCircuitNode::ConnectionsRes DiodeCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact >= getContactCount())
        return {};

    // We always go from 0 to 1 unless invertDir
    if(source.nodeContact == 0 && !invertDir)
    {
        // Connect to central positive
        CableItemFlags dest;
        dest.cable.cable = mContacts.at(1).cable;
        dest.cable.side = mContacts.at(1).cableSide;
        dest.nodeContact = 1;
        dest.cable.pole = source.cable.pole;

        return {dest};
    }
    else if(source.nodeContact == 1 && invertDir)
    {
        CableItemFlags dest;
        dest.cable.cable = mContacts.at(0).cable;
        dest.cable.side = mContacts.at(0).cableSide;
        dest.nodeContact = 0;
        dest.cable.pole = source.cable.pole;

        return {dest};
    }

    return {};
}

void DiodeCircuitNode::addCircuit(ElectricCircuit *circuit)
{
    CircuitList& circuitList = getCircuits(circuit->type());

    // A circuit may pass 2 times on same node
    // But we add it only once
    if(circuitList.contains(circuit))
        return;

    bool updateNeeded = false;

    const auto items = circuit->getNode(this);
    for(int i = 0; i < items.size(); i++)
    {
        const NodeItem& item = items.at(i);

        if(item.fromContact == 0 && item.toContact == 1)
        {
            if(circuit->type() == CircuitType::Closed)
            {
                updateNeeded |= mPassingClosedCircuitsCount == 0;
                mPassingClosedCircuitsCount++;
            }
            else
            {
                updateNeeded |= mPassingOpenCircuitsCount == 0;
                mPassingOpenCircuitsCount++;
            }
        }
    }

    AbstractCircuitNode::addCircuit(circuit);

    if(updateNeeded)
        emit circuitsChanged();
}

void DiodeCircuitNode::partialRemoveCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    AbstractCircuitNode::partialRemoveCircuit(circuit, items);

    bool updateNeeded = false;

    for(int i = 0; i < items.size(); i++)
    {
        const NodeItem& item = items.at(i);

        if(item.fromContact == 0 && item.toContact == 1)
        {
            if(circuit->type() == CircuitType::Closed)
            {
                updateNeeded |= mPassingClosedCircuitsCount == 1;
                mPassingClosedCircuitsCount--;
            }
            else
            {
                updateNeeded |= mPassingOpenCircuitsCount == 1;
                mPassingOpenCircuitsCount--;
            }
        }
    }

    if(updateNeeded)
        emit circuitsChanged();
}

QString DiodeCircuitNode::nodeType() const
{
    return NodeType;
}
