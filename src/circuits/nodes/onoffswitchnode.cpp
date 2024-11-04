/**
 * src/circuits/nodes/onoffswitchnode.cpp
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

#include "onoffswitchnode.h"

#include "../electriccircuit.h"

#include "../../views/modemanager.h"

OnOffSwitchNode::OnOffSwitchNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, false, parent}
{
    // 2 sides
    mContacts.append(NodeContact("11", "12"));
    mContacts.append(NodeContact("21", "22"));
}

QVector<CableItem> OnOffSwitchNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact > 1))
        return {};

    if(!m_isOn)
        return {};

    const NodeContact& sourceContact = mContacts.at(source.nodeContact);
    if(sourceContact.getType(source.cable.pole) == ContactType::NotConnected)
        return {};

    const NodeContact& otherContact = mContacts.at(source.nodeContact == 0 ? 1 : 0);

    CableItem other;
    other.cable.cable = otherContact.cable;
    other.cable.side = otherContact.cableSide;
    other.nodeContact = source.nodeContact == 0 ? 1 : 0;
    other.cable.pole = source.cable.pole;

    switch (otherContact.getType(source.cable.pole))
    {
    default:
    case ContactType::Connected:
    case ContactType::NotConnected:
        if(m_isOn)
            return {other};
        return {};
    case ContactType::Passthrough:
        return {other};
    }

    return {};
}

QString OnOffSwitchNode::nodeType() const
{
    return NodeType;
}

bool OnOffSwitchNode::isOn() const
{
    if(m_isOn && modeMgr()->mode() == FileMode::Editing)
        return false; // Act as Off during Editing
    return m_isOn;
}

void OnOffSwitchNode::setOn(bool newOn)
{
    if(modeMgr()->mode() == FileMode::Editing)
        return; // Prevent turning on during editing

    if (m_isOn == newOn)
        return;
    m_isOn = newOn;
    emit isOnChanged(m_isOn);

    if(m_isOn)
    {
        ElectricCircuit::createCircuitsFromOtherNode(this);
    }
    else
    {
        bool hadCircuits = hasCircuits(CircuitType::Closed) || hasCircuits(CircuitType::Open);

        // Disable circuits
        const CircuitList closedCopy = getCircuits(CircuitType::Closed);
        disableCircuits(closedCopy, this);

        const CircuitList openCopy = getCircuits(CircuitType::Open);
        truncateCircuits(openCopy, this);

        if(hadCircuits)
        {
            ElectricCircuit::defaultReachNextOpenCircuit(this);
        }
    }
}
