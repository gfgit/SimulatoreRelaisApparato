/**
 * src/nodes/circuitcable.cpp
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

#include "circuitcable.h"

#include "abstractcircuitnode.h"

#include "../core/electriccircuit.h"

CircuitCable::CircuitCable(QObject *parent)
    : QObject{parent}
{

}

CircuitCable::~CircuitCable()
{
    Q_ASSERT(getCircuits(CircuitType::Closed,
                         CircuitPole::First).isEmpty());
    Q_ASSERT(getCircuits(CircuitType::Closed,
                         CircuitPole::Second).isEmpty());
    Q_ASSERT(getCircuits(CircuitType::Open,
                         CircuitPole::First).isEmpty());
    Q_ASSERT(getCircuits(CircuitType::Open,
                         CircuitPole::Second).isEmpty());

    // Detach all nodes
    if(mNodeA.node)
    {
        CableItem item;
        item.cable.cable = this;
        item.cable.side = CableSide::A;
        item.nodeContact = mNodeA.nodeContact;

        item.cable.pole = CircuitPole::First;
        if(mNodeA.node->getContactType(mNodeA.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeA.node->detachCable(item);
        }

        item.cable.pole = CircuitPole::Second;
        if(mNodeA.node->getContactType(mNodeA.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeA.node->detachCable(item);
        }
    }

    if(mNodeB.node)
    {
        CableItem item;
        item.cable.cable = this;
        item.cable.side = CableSide::B;
        item.nodeContact = mNodeB.nodeContact;

        item.cable.pole = CircuitPole::First;
        if(mNodeB.node->getContactType(mNodeB.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeB.node->detachCable(item);
        }

        item.cable.pole = CircuitPole::Second;
        if(mNodeB.node->getContactType(mNodeB.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            mNodeB.node->detachCable(item);
        }
    }
}

CircuitCable::Mode CircuitCable::mode() const
{
    return mMode;
}

void CircuitCable::setMode(Mode newMode)
{
    if (mMode == newMode)
        return;
    mMode = newMode;

    emit modeChanged(mMode);
}

void CircuitCable::addCircuit(ElectricCircuit *circuit, CircuitPole pole)
{
    const CablePower oldPower = powered();

    CircuitList& circuitList = getCircuits(circuit->type(),
                                           pole);

    if(circuitList.contains(circuit))
        return; // A circuit may pass 2 times on same item
    circuitList.append(circuit);

    const CablePower newPower = powered();
    if(oldPower != newPower)
        emit powerChanged(newPower);
}

void CircuitCable::removeCircuit(ElectricCircuit *circuit)
{
    const CablePower oldPower = powered();

    CircuitList& circuitList1 = getCircuits(circuit->type(),
                                            CircuitPole::First);
    CircuitList& circuitList2 = getCircuits(circuit->type(),
                                            CircuitPole::Second);

    Q_ASSERT(circuitList1.contains(circuit) ||
             circuitList2.contains(circuit));

    circuitList1.removeOne(circuit);
    circuitList2.removeOne(circuit);

    const CablePower newPower = powered();
    if(oldPower != newPower)
        emit powerChanged(newPower);
}

void CircuitCable::setNode(CableSide s, CableEnd node)
{
    switch (s)
    {
    case CableSide::A:
        mNodeA = node;
        break;
    case CableSide::B:
        mNodeB = node;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    emit nodesChanged();
}

CablePower CircuitCable::powered()
{
    // Cable is powered if there are
    // closed circuits passing on it

    const bool firstOn = !getCircuits(CircuitType::Closed,
                                      CircuitPole::First).isEmpty();
    const bool secondOn = !getCircuits(CircuitType::Closed,
                                       CircuitPole::Second).isEmpty();

    if(firstOn)
    {
        if(secondOn)
            return CablePowerPole::Both | CircuitType::Closed;
        return CablePowerPole::First | CircuitType::Closed;
    }
    else if(secondOn)
    {
        return CablePowerPole::Second | CircuitType::Closed;
    }

    const bool firstOpenOn = !getCircuits(CircuitType::Open,
                                          CircuitPole::First).isEmpty();
    const bool secondOpenOn = !getCircuits(CircuitType::Open,
                                           CircuitPole::Second).isEmpty();

    if(firstOpenOn)
    {
        if(secondOpenOn)
            return CablePowerPole::Both | CircuitType::Open;
        return CablePowerPole::First | CircuitType::Open;
    }
    else if(secondOpenOn)
    {
        return CablePowerPole::Second | CircuitType::Open;
    }

    return CablePowerPole::None | CircuitType::Open;
}
