/**
 * src/nodes/simplecircuitnode.cpp
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

#include "simplecircuitnode.h"

#include "../core/electriccircuit.h"

#include <QJsonObject>

SimpleCircuitNode::SimpleCircuitNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 4 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<CableItem> SimpleCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact >= getContactCount())
        return {};

    if(!isContactEnabled(source.nodeContact))
        return {};

    QVector<CableItem> result;
    result.reserve(3);

    for(int i = 0; i < 4; i++) // Loop contacts of same polarity
    {
        if(i == source.nodeContact)
            continue; // Skip self

        if(!isContactEnabled(i))
            continue;

        CableItem dest;
        dest.cable.cable = mContacts.at(i).cable;
        dest.cable.side = mContacts.at(i).cableSide;
        dest.nodeContact = i;
        dest.cable.pole = source.cable.pole;

        result.append(dest);
    }

    return result;
}

void SimpleCircuitNode::setDisabledContact(int val)
{
    Q_ASSERT(val >= 0 && val < 4);
    if(mDisabledContact == val)
        return;
    mDisabledContact = val;
    emit shapeChanged();

    // Commmon cannot be disabled
    if(mDisabledContact > 0)
    {
        // Remove circuits and detach cable
        // No need to add circuits to previous disabled contact
        // Since it will not have cable attached
        if(hasCircuit(mDisabledContact) > 0)
        {
            // Disable all circuits passing on disabled contact
            const CircuitList closedCopy = getCircuits(CircuitType::Closed);
            disableCircuits(closedCopy, this, mDisabledContact);

            const CircuitList openCopy = getCircuits(CircuitType::Open);
            truncateCircuits(openCopy, this, mDisabledContact);
        }

        detachCable(mDisabledContact);
    }
}

bool SimpleCircuitNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    int val = obj.value("disabledContact").toInt();
    if(val < 0 || val >= 4)
        return false;

    setDisabledContact(val);
    return true;
}

void SimpleCircuitNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["disabledContact"] = disabledContact();
}

QString SimpleCircuitNode::nodeType() const
{
    return NodeType;
}
