/**
 * src/circuits/nodes/resistornode.cpp
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

#include "resistornode.h"

ResistorNode::ResistorNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 2 sides
    mContacts.append(NodeContact("11", "12"));
    mContacts.append(NodeContact("21", "22"));
}

AbstractCircuitNode::ConnectionsRes ResistorNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact > 1))
        return {};

    const NodeContact& otherContact = mContacts.at(source.nodeContact == 0 ? 1 : 0);

    CableItemFlags other;
    other.cable.cable = otherContact.cable;
    other.cable.side = otherContact.cableSide;
    other.nodeContact = source.nodeContact == 0 ? 1 : 0;
    other.cable.pole = source.cable.pole;

    other.flags = CircuitFlags::Resistor;

    return {other};
}

QString ResistorNode::nodeType() const
{
    return NodeType;
}
