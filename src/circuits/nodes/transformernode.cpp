/**
 * src/circuits/nodes/transformernode.cpp
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

#include "transformernode.h"

#include "../electriccircuit.h"

#include "../../views/modemanager.h"

#include <QCoreApplication>
#include <QEvent>

class TransformerNodeEnableEvent : public QEvent
{
public:
    static const QEvent::Type _Type = QEvent::Type(QEvent::User + 2);

    TransformerNodeEnableEvent() :
        QEvent(_Type)
    {
    }
};


TransformerNode::TransformerNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 2 sides
    mContacts.append(NodeContact("1", "2"));
    mContacts.append(NodeContact("3", "4"));
}

bool TransformerNode::event(QEvent *e)
{
    if(e->type() == TransformerNodeEnableEvent::_Type)
    {
        e->accept();
        updateSourceState();
        return true;
    }

    return AbstractCircuitNode::event(e);
}

void TransformerNode::addCircuit(ElectricCircuit *circuit)
{
    const AnyCircuitType oldSourceState = hasAnyCircuit(1);

    AbstractCircuitNode::addCircuit(circuit);
    const bool shouldEnable = hasCircuit(0, CircuitType::Closed);

    bool changed = false;
    if(shouldEnable != reallyEnabled)
    {
        QCoreApplication::postEvent(this, new TransformerNodeEnableEvent);
        changed = true;
    }

    const AnyCircuitType sourceState = hasAnyCircuit(1);
    if(oldSourceState != sourceState)
        changed = true;

    if(changed)
        emit circuitsChanged();
}

void TransformerNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const AnyCircuitType oldSourceState = hasAnyCircuit(1);

    AbstractCircuitNode::removeCircuit(circuit, items);
    const bool shouldEnable = hasCircuit(0, CircuitType::Closed);

    bool changed = false;
    if(shouldEnable != reallyEnabled)
    {
        QCoreApplication::postEvent(this, new TransformerNodeEnableEvent);
        changed = true;
    }

    const AnyCircuitType sourceState = hasAnyCircuit(1);
    if(oldSourceState != sourceState)
        changed = true;

    if(changed)
        emit circuitsChanged();
}

void TransformerNode::partialRemoveCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const AnyCircuitType oldSourceState = hasAnyCircuit(1);

    AbstractCircuitNode::partialRemoveCircuit(circuit, items);
    const bool shouldEnable = hasCircuit(0, CircuitType::Closed);

    bool changed = false;
    if(shouldEnable != reallyEnabled)
    {
        QCoreApplication::postEvent(this, new TransformerNodeEnableEvent);
        changed = true;
    }

    const AnyCircuitType sourceState = hasAnyCircuit(1);
    if(oldSourceState != sourceState)
        changed = true;

    if(changed)
        emit circuitsChanged();
}

AbstractCircuitNode::ConnectionsRes TransformerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 && source.nodeContact != 1)
        return {};

    if(!mContacts.at(0).cable)
        return {};

    if(source.nodeContact == 1)
    {
        // Make dependant circuits end here
        return {};
    }

    // Close the primary circuit
    CableItemFlags dest;
    dest.cable.cable = mContacts.at(0).cable;
    dest.cable.side = mContacts.at(0).cableSide;
    dest.nodeContact = 0;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

QString TransformerNode::nodeType() const
{
    return NodeType;
}

bool TransformerNode::isSourceNode(bool onlyCurrentState, int nodeContact) const
{
    Q_UNUSED(onlyCurrentState);
    return nodeContact == 1 || nodeContact == NodeItem::InvalidContact;
}

bool TransformerNode::isSourceEnabled() const
{
    if(enabled && modeMgr()->mode() == FileMode::Editing)
        return false; // Act as Off during Editing

    // Return true if explicitly enabled and has source circuit
    return enabled && hasCircuit(0, CircuitType::Closed);
}

void TransformerNode::setSourceEnabled(bool newEnabled)
{
    if(modeMgr()->mode() == FileMode::Editing && newEnabled)
        return; // Prevent enabling during editing

    if (enabled == newEnabled)
        return;
    enabled = newEnabled;

    updateSourceState();
}

void TransformerNode::updateSourceState()
{
    const bool shouldEnable = enabled && hasCircuit(0, CircuitType::Closed)
            && modeMgr()->mode() != FileMode::Editing;

    if (reallyEnabled == shouldEnable)
        return;
    reallyEnabled = shouldEnable;

    if(reallyEnabled)
    {
        CircuitPole pole = CircuitPole::First;
        if(hasExitCircuitOnPole(0, CircuitPole::First))
            pole = CircuitPole::Second;

        ElectricCircuit::createCircuitsFromPowerNode(this, pole, 1);
    }
    else
    {
        // Disable circuits
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this);
    }
}
