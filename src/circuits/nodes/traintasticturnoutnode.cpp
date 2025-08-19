/**
 * src/circuits/nodes/traintasticturnoutnode.cpp
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

#include "traintasticturnoutnode.h"


#include "../../objects/abstractsimulationobjectmodel.h"
#include "../../objects/traintastic/traintasticturnoutobj.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

TraintasticTurnoutNode::TraintasticTurnoutNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode(mgr, true, parent)
{
    // 2 sides: N, R
    mContacts.append(NodeContact("41", "42"));
    mContacts.append(NodeContact("43", "44"));
}

TraintasticTurnoutNode::~TraintasticTurnoutNode()
{
    setTurnout(nullptr);
}

void TraintasticTurnoutNode::addCircuit(ElectricCircuit *circuit)
{
    AbstractCircuitNode::addCircuit(circuit);
    updateState();
}

void TraintasticTurnoutNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    AbstractCircuitNode::removeCircuit(circuit, items);
    updateState();
}

AbstractCircuitNode::ConnectionsRes TraintasticTurnoutNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact > 1)
        return {};

    if(!turnout())
        return{};

    // Close the circuit
    CableItemFlags dest;
    dest.cable.cable = mContacts.at(source.nodeContact).cable;
    dest.cable.side = mContacts.at(source.nodeContact).cableSide;
    dest.nodeContact = source.nodeContact;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

bool TraintasticTurnoutNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    const QString turnoutName = obj.value("turnout").toString();
    auto model = modeMgr()->modelForType(TraintasticTurnoutObj::Type);

    if(model)
        setTurnout(static_cast<TraintasticTurnoutObj *>(model->getObjectByName(turnoutName)));
    else
        setTurnout(nullptr);

    return true;
}

void TraintasticTurnoutNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["turnout"] = mTurnout ? mTurnout->name() : QString();
}

void TraintasticTurnoutNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty butProp;
    butProp.name = "turnout";
    butProp.prettyName = tr("Turnout");
    butProp.types = {TraintasticTurnoutObj::Type};
    result.append(butProp);
}

QString TraintasticTurnoutNode::nodeType() const
{
    return NodeType;
}

TraintasticTurnoutObj *TraintasticTurnoutNode::turnout() const
{
    return mTurnout;
}

void TraintasticTurnoutNode::setTurnout(TraintasticTurnoutObj *newTurnout)
{
    if (mTurnout == newTurnout)
        return;

    if(mTurnout)
    {
        mTurnout->setNode(nullptr);
    }

    mTurnout = newTurnout;

    if(mTurnout)
    {
        mTurnout->setNode(this);
    }

    emit turnoutChanged(mTurnout);
    modeMgr()->setFileEdited();

    updateState();
}

void TraintasticTurnoutNode::updateState()
{
    if(!turnout())
        return;

    const bool activeN = hasCircuit(0);
    const bool activeR = hasCircuit(1);

    if(activeN && !activeR)
        mTurnout->setActive(true, false);
    else if(!activeN && activeR)
        mTurnout->setActive(true, true);
    else
        mTurnout->setActive(false, false);
}
