/**
 * src/circuits/nodes/circuitcable.cpp
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

#include "../electriccircuit.h"

#include "../../views/modemanager.h"

CircuitCable::CircuitCable(ModeManager *mgr, QObject *parent)
    : QObject{parent}
    , mModeMgr(mgr)
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
    Q_ASSERT(mCircuitsWithFlags == 0);

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
    modeMgr()->setFileEdited();
}

void CircuitCable::addCircuit(ElectricCircuit *circuit, CircuitPole pole)
{
    const CablePower oldPower = powered();

    CircuitList& circuitList = getCircuits(circuit->type(),
                                           pole);

    // Since pole lists are different
    // a circuit may pass only once per list
    Q_ASSERT(!circuitList.contains(circuit));
    circuitList.append(circuit);

    if(circuit->flags() != getFlags(circuit->type(), pole))
        updateCircuitFlags(circuit->type(), pole);

    if(circuit->flags() != CircuitFlags::None)
        mCircuitsWithFlags++;

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

    bool isFirst = circuitList1.removeOne(circuit);
    bool isSecond = circuitList2.removeOne(circuit);

    if(mCircuitsWithFlags > 0)
    {
        if(isFirst)
            updateCircuitFlags(circuit->type(), CircuitPole::First);
        if(isSecond)
            updateCircuitFlags(circuit->type(), CircuitPole::Second);
    }

    if(circuit->flags() != CircuitFlags::None)
    {
        mCircuitsWithFlags--;
        if(isFirst && isSecond)
            mCircuitsWithFlags--;
    }
    Q_ASSERT(mCircuitsWithFlags >= 0);

    const CablePower newPower = powered();
    if(oldPower != newPower)
        emit powerChanged(newPower);
}

void CircuitCable::circuitAddedRemovedFlags(ElectricCircuit *circuit, bool add)
{
    CircuitList& circuitList1 = getCircuits(circuit->type(),
                                            CircuitPole::First);
    CircuitList& circuitList2 = getCircuits(circuit->type(),
                                            CircuitPole::Second);

    bool isFirst = circuitList1.contains(circuit);
    bool isSecond = circuitList2.contains(circuit);

    Q_ASSERT(isFirst || isSecond);

    if(mCircuitsWithFlags > 0)
    {
        if(isFirst)
            updateCircuitFlags(circuit->type(), CircuitPole::First);
        if(isSecond)
            updateCircuitFlags(circuit->type(), CircuitPole::Second);
    }

    if(add)
    {
        mCircuitsWithFlags++;
        if(isFirst && isSecond)
            mCircuitsWithFlags++;
    }
    else
    {
        mCircuitsWithFlags--;
        if(isFirst && isSecond)
            mCircuitsWithFlags--;
    }
    Q_ASSERT(mCircuitsWithFlags >= 0);
}

void CircuitCable::updateCircuitFlags(CircuitType type, CircuitPole pole)
{
    CircuitFlags newFlags = CircuitFlags::None;

    const CircuitList& circuitList = getCircuits(type, pole);
    if(!circuitList.isEmpty())
        newFlags = circuitList.first()->flags();

    for(ElectricCircuit *circuit : circuitList)
        newFlags = newFlags & circuit->flags();

    CircuitFlags &curFlags = getFlags(type, pole);
    if(newFlags != curFlags)
    {
        curFlags = newFlags;
        emit powerChanged(powered());
    }
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

CablePower CircuitCable::powered() const
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

CircuitFlags CircuitCable::getFlags() const
{
    const bool firstOn = !getCircuits(CircuitType::Closed,
                                      CircuitPole::First).isEmpty();
    const bool secondOn = !getCircuits(CircuitType::Closed,
                                       CircuitPole::Second).isEmpty();

    if(firstOn)
    {
        if(secondOn)
            return mFlagsFirstClosed & mFlagsSecondClosed;
        return mFlagsFirstClosed;
    }
    else if(secondOn)
    {
        return mFlagsSecondClosed;
    }

    const bool firstOpenOn = !getCircuits(CircuitType::Open,
                                          CircuitPole::First).isEmpty();
    const bool secondOpenOn = !getCircuits(CircuitType::Open,
                                           CircuitPole::Second).isEmpty();

    if(firstOpenOn)
    {
        if(secondOpenOn)
            return mFlagsFirstOpen & mFlagsSecondOpen;
        return mFlagsFirstOpen;
    }
    else if(secondOpenOn)
    {
        return mFlagsSecondOpen;
    }

    return CircuitFlags::None;
}
