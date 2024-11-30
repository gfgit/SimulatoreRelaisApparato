/**
 * src/circuits/nodes/bifilarizatornode.cpp
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

#include "bifilarizatornode.h"

BifilarizatorNode::BifilarizatorNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, false, parent}
{
    // 3 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<CableItem> BifilarizatorNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact >= getContactCount())
        return {};

    // For bifilar cables we use only First pole, so reject Second pole
    if(source.nodeContact == 0)
    {
        if(source.cable.pole != CircuitPole::First)
            return {};

        // Connect to central positive
        CableItem centralPositive;
        centralPositive.cable.cable = mContacts.at(1).cable;
        centralPositive.cable.side = mContacts.at(1).cableSide;
        centralPositive.nodeContact = 1;
        centralPositive.cable.pole = CircuitPole::First; // Positive

        return {centralPositive};
    }
    else if(source.nodeContact == 1)
    {
        // Go to 0 or 2 based on unifilar polarity
        const int destContact = (source.cable.pole == CircuitPole::First ? 0 : 2);

        // Connect to first pole of out cable
        CableItem dest;
        dest.cable.cable = mContacts.at(destContact).cable;
        dest.cable.side = mContacts.at(destContact).cableSide;
        dest.nodeContact = destContact;
        dest.cable.pole = CircuitPole::First;

        return {dest};
    }
    else if(source.nodeContact == 2)
    {
        if(source.cable.pole != CircuitPole::First)
            return {};

        // Connect to central negative
        CableItem centralNegative;
        centralNegative.cable.cable = mContacts.at(1).cable;
        centralNegative.cable.side = mContacts.at(1).cableSide;
        centralNegative.nodeContact = 1;
        centralNegative.cable.pole = CircuitPole::Second; // Negative

        return {centralNegative};
    }

    return {};
}

QString BifilarizatorNode::nodeType() const
{
    return NodeType;
}
