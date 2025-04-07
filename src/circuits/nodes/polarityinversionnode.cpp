/**
 * src/circuits/nodes/polarityinversionnode.cpp
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

#include "polarityinversionnode.h"

PolarityInversionNode::PolarityInversionNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, false, parent}
{
    // 2 sides
    mContacts.append(NodeContact("1", "2"));
    mContacts.append(NodeContact("3", "4"));
}

AbstractCircuitNode::ConnectionsRes PolarityInversionNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 && source.nodeContact != 1)
        return {};

    const int otherContactIdx = source.nodeContact == 0 ? 1 : 0;

    // Invert polarity
    CableItem dest;
    dest.cable.cable = mContacts.at(otherContactIdx).cable;
    dest.cable.side = mContacts.at(otherContactIdx).cableSide;
    dest.nodeContact = otherContactIdx;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

QString PolarityInversionNode::nodeType() const
{
    return NodeType;
}
