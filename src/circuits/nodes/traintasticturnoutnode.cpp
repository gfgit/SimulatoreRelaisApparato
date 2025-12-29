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
#include "../../objects/traintastic/traintasticspawnobj.h"
#include "../../objects/traintastic/traintasticauxsignalobject.h"

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
    setSpawn(nullptr);
    setAuxSignal(nullptr);
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

    if(!turnout() && !spawn() && !auxSignal())
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
    auto turnoutModel = modeMgr()->modelForType(TraintasticTurnoutObj::Type);

    if(turnoutModel)
        setTurnout(static_cast<TraintasticTurnoutObj *>(turnoutModel->getObjectByName(turnoutName)), false);
    else
        setTurnout(nullptr);

    const QString spawnName = obj.value("spawn").toString();
    auto spawnModel = modeMgr()->modelForType(TraintasticSpawnObj::Type);

    if(spawnModel)
        setSpawn(static_cast<TraintasticSpawnObj *>(spawnModel->getObjectByName(spawnName)), false);
    else
        setSpawn(nullptr);

    const QString auxSignalName = obj.value("aux_signal").toString();
    auto auxSignalModel = modeMgr()->modelForType(TraintasticAuxSignalObject::Type);

    if(auxSignalModel)
        setAuxSignal(static_cast<TraintasticAuxSignalObject *>(auxSignalModel->getObjectByName(auxSignalName)));
    else
        setAuxSignal(nullptr);

    return true;
}

void TraintasticTurnoutNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["turnout"] = mTurnout ? mTurnout->name() : QString();

    if(mSpawn)
        obj["spawn"] = mSpawn->name();
    if(mAuxSignal)
        obj["aux_signal"] = mAuxSignal->name();
}

void TraintasticTurnoutNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty objProp;
    objProp.name = "turnout";
    objProp.prettyName = tr("Turnout");
    objProp.types = {TraintasticTurnoutObj::Type};
    result.append(objProp);

    objProp.name = "spawn";
    objProp.prettyName = tr("Spawn");
    objProp.types = {TraintasticSpawnObj::Type};
    result.append(objProp);

    objProp.name = "aux_signal";
    objProp.prettyName = tr("Aux Signal");
    objProp.types = {TraintasticAuxSignalObject::Type};
    result.append(objProp);
}

QString TraintasticTurnoutNode::nodeType() const
{
    return NodeType;
}

TraintasticTurnoutObj *TraintasticTurnoutNode::turnout() const
{
    return mTurnout;
}

bool TraintasticTurnoutNode::setTurnout(TraintasticTurnoutObj *newTurnout, bool steal)
{
    if (mTurnout == newTurnout)
        return true;

    if(newTurnout && newTurnout->getNode() && !steal)
        return false;

    if(newTurnout)
    {
        setSpawn(nullptr);
        setAuxSignal(nullptr);
    }

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
    emit shapeChanged();
    modeMgr()->setFileEdited();

    updateState();
    return true;
}

TraintasticSpawnObj *TraintasticTurnoutNode::spawn() const
{
    return mSpawn;
}

bool TraintasticTurnoutNode::setSpawn(TraintasticSpawnObj *newSpawn, bool steal)
{
    if (mSpawn == newSpawn)
        return true;

    if(newSpawn && newSpawn->getNode() && !steal)
        return false;

    if(newSpawn)
    {
        setTurnout(nullptr);
        setAuxSignal(nullptr);
    }

    if(mSpawn)
    {
        mSpawn->setNode(nullptr);
    }

    mSpawn = newSpawn;

    if(mSpawn)
    {
        mSpawn->setNode(this);
    }

    emit spawnChanged(mSpawn);
    emit shapeChanged();
    modeMgr()->setFileEdited();

    updateState();
    return true;
}

void TraintasticTurnoutNode::updateState()
{
    const bool activeN = hasCircuit(0);
    const bool activeR = hasCircuit(1);

    if(turnout())
    {
        if(activeN && !activeR)
            mTurnout->setActive(true, false);
        else if(!activeN && activeR)
            mTurnout->setActive(true, true);
        else
            mTurnout->setActive(false, false);
    }
    else if(spawn())
    {
        mSpawn->setActive(activeN);
    }
    else if(auxSignal())
    {
        if(activeN && !activeR)
            mAuxSignal->setMotorState(TraintasticAuxSignalObject::MotorState::GoFowrard);
        else if(!activeN && activeR)
            mAuxSignal->setMotorState(TraintasticAuxSignalObject::MotorState::GoBackwards);
        else
            mAuxSignal->setMotorState(TraintasticAuxSignalObject::MotorState::Idle);
    }
}

TraintasticAuxSignalObject *TraintasticTurnoutNode::auxSignal() const
{
    return mAuxSignal;
}

void TraintasticTurnoutNode::setAuxSignal(TraintasticAuxSignalObject *newAuxSignal)
{
    if(mAuxSignal == newAuxSignal)
        return;

    if(mAuxSignal)
    {
        setSpawn(nullptr);
        setTurnout(nullptr);
    }

    if(mAuxSignal)
    {
        disconnect(mAuxSignal, &TraintasticAuxSignalObject::destroyed,
                   this, &TraintasticTurnoutNode::onAuxSignalsDestroyed);
        mAuxSignal->setMotorState(TraintasticAuxSignalObject::MotorState::Idle);
    }

    mAuxSignal = newAuxSignal;

    if(mAuxSignal)
    {
        connect(mAuxSignal, &TraintasticAuxSignalObject::destroyed,
                this, &TraintasticTurnoutNode::onAuxSignalsDestroyed);
    }

    emit auxSignalChanged(mAuxSignal);
    emit shapeChanged();
    modeMgr()->setFileEdited();

    updateState();
    return;
}

void TraintasticTurnoutNode::onAuxSignalsDestroyed(QObject *obj)
{
    if(obj == mAuxSignal)
        setAuxSignal(nullptr);
}
