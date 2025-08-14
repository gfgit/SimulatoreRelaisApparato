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

#include <QJsonObject>

TraintasticSensorObj::TraintasticSensorObj(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject(m)
{

}

TraintasticSensorObj::~TraintasticSensorObj()
{
    setChannel(-1);
    setAddress(-1);
}

QString TraintasticSensorObj::getType() const
{
    return Type;
}

bool TraintasticSensorObj::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    SensorType newType = SensorType::Generic;
    if(obj.value("sensor_type").toInt() != 0)
        newType = SensorType::TurnoutFeedback;
    setSensorType(newType);

    setChannel(obj.value("channel").toInt(-1));
    setAddress(obj.value("address").toInt(-1));

    return true;
}

void TraintasticSensorObj::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["sensor_type"] = int(mSensorType);
    obj["channel"] = mChannel;
    obj["address"] = mChannel;
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

    setChannel(-1);
    setAddress(-1);
    mSensorType = newSensorType;

    emit settingsChanged(this);
}

void TraintasticSensorObj::setState(int newState)
{
    if(newState == mState)
        return;

    mState = newState;
    emit stateChanged(this);
}
