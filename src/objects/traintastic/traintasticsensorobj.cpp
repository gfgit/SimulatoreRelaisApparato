/**
 * src/objects/traintastic/traintasticsensorobj.cpp
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

#include "traintasticsensorobj.h"
#include "../abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"
#include "../../network/traintastic-simulator/traintasticsimmanager.h"

#include "../../circuits/nodes/traintasticsensornode.h"

#include "traintasticturnoutobj.h"

#include <QJsonObject>

TraintasticSensorObj::TraintasticSensorObj(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject(m)
{

}

TraintasticSensorObj::~TraintasticSensorObj()
{
    setChannel(-1);
    setAddress(-1);
    setShuntTurnout(nullptr);

    auto contactNodes = mContactNodes;
    for(TraintasticSensorNode *c : contactNodes)
    {
        c->setSensor(nullptr);
    }
}

QString TraintasticSensorObj::getType() const
{
    return Type;
}

bool TraintasticSensorObj::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase == LoadPhase::Creation)
    {
        const SensorType newType = SensorType(obj.value("sensor_type").toInt(int(SensorType::Generic)));
        setSensorType(newType);
        setDefaultOffState(obj.value("off_state").toInt(1));

        setChannel(obj.value("channel").toInt(InvalidChannel));
        setAddress(obj.value("address").toInt(InvalidAddress));
    }
    else
    {
        const QString turnoutName = obj.value("shunt_turnout").toString();
        auto model_ = model()->modeMgr()->modelForType(TraintasticTurnoutObj::Type);

        if(model_)
            setShuntTurnout(static_cast<TraintasticTurnoutObj *>(model_->getObjectByName(turnoutName)));
        else
            setShuntTurnout(nullptr);

        // Set initial state
        onTraintasticDisconnected();
    }

    return true;
}

void TraintasticSensorObj::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["sensor_type"] = int(mSensorType);
    obj["channel"] = mChannel;
    obj["address"] = mAddress;
    obj["off_state"] = mDefaultOffState;
    obj["shunt_turnout"] = mShuntTurnout ? mShuntTurnout->name() : QString();
}

int TraintasticSensorObj::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int nodesCount = AbstractSimulationObject::getReferencingNodes(result);

    nodesCount += mContactNodes.size();

    if(result)
    {
        for(auto item : mContactNodes)
            result->append(item);
    }

    return nodesCount;
}

bool TraintasticSensorObj::setChannel(int newChannel)
{
    if(mChannel == newChannel)
        return true;

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    if(!mgr->setSensorChannel(this, newChannel))
        return false;

    mChannel = newChannel;
    emit settingsChanged(this);
    return true;
}

bool TraintasticSensorObj::setAddress(int newAddress)
{
    if(mAddress == newAddress)
        return true;

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    if(!mgr->setSensorAddress(this, newAddress))
        return false;

    mAddress = newAddress;
    emit settingsChanged(this);
    return true;
}

void TraintasticSensorObj::setSensorType(SensorType newSensorType)
{
    if(mSensorType == newSensorType)
        return;

    // Invalidate before changing type
    setChannel(-1);
    setAddress(-1);
    mSensorType = newSensorType;

    if(mSensorType != SensorType::TurnoutFeedback)
        setShuntTurnout(nullptr);

    emit settingsChanged(this);
}

void TraintasticSensorObj::setState(int newState)
{
    if(newState == mState)
        return;

    mState = newState;
    emit stateChanged(this);

    for(TraintasticSensorNode *c : mContactNodes)
    {
        c->refreshContactState();
    }
}

void TraintasticSensorObj::addContactNode(TraintasticSensorNode *c)
{
    Q_ASSERT_X(!mContactNodes.contains(c),
               "addContactNode", "already added");

    mContactNodes.append(c);

    emit nodesChanged(this);
}

void TraintasticSensorObj::removeContactNode(TraintasticSensorNode *c)
{
    Q_ASSERT_X(mContactNodes.contains(c),
               "removeContactNode", "not registered");
    Q_ASSERT_X(c->sensor() == this,
               "removeContactNode", "sensor does not match");

    mContactNodes.removeOne(c);

    emit nodesChanged(this);
}

void TraintasticSensorObj::onTraintasticDisconnected()
{
    if(sensorType() == SensorType::TurnoutFeedback && mShuntTurnout)
        setState(mShuntTurnout ? mShuntTurnout->state() : TraintasticTurnoutObj::State::Unknown);
    else
        setState(defaultOffState());
}

void TraintasticSensorObj::setDefaultOffState(int newDefaultOffState)
{
    if(mDefaultOffState == newDefaultOffState)
        return;

    mDefaultOffState = newDefaultOffState;
    emit settingsChanged(this);

    setState(mDefaultOffState);
}

TraintasticTurnoutObj *TraintasticSensorObj::shuntTurnout() const
{
    return mShuntTurnout;
}

bool TraintasticSensorObj::setShuntTurnout(TraintasticTurnoutObj *newShuntTurnout)
{
    if(mShuntTurnout == newShuntTurnout)
        return true;

    if(newShuntTurnout && (newShuntTurnout->address() != address() || newShuntTurnout->channel() != channel()))
    {
        const int oldChannel = channel();
        const int oldAddress = address();
        setChannel(InvalidChannel);

        if(!setAddress(newShuntTurnout->address()))
        {
            setChannel(oldChannel);
            return false;
        }

        if(!setChannel(newShuntTurnout->channel()))
        {
            setAddress(oldAddress);
            setChannel(oldChannel);
            return false;
        }
    }

    if(mShuntTurnout)
    {
        mShuntTurnout->setSensor(nullptr);
    }

    mShuntTurnout = newShuntTurnout;

    if(mShuntTurnout)
    {
        mShuntTurnout->setSensor(this);
    }

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    if(!mgr->isConnected())
    {
        setState(mShuntTurnout ? mShuntTurnout->state() : TraintasticTurnoutObj::State::Unknown);
    }

    emit settingsChanged(this);
    return true;
}


