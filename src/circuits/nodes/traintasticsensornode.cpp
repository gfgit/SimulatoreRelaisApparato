/**
 * src/circuits/nodes/traintasticsensornode.cpp
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

#include "traintasticsensornode.h"

#include "../../objects/abstractsimulationobjectmodel.h"
#include "../../objects/traintastic/traintasticsensorobj.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

TraintasticSensorNode::TraintasticSensorNode(ModeManager *mgr, QObject *parent)
    : AbstractDeviatorNode{mgr, parent}
{
    // 3 sides
    // Common
    // Pressed (Deviator Up contact)
    // Normal  (Deviator Down contact)

    setSwapContactState(false);
    setAllowSwap(true);

    setHasCentralConnector(false);

    // Default state
    setContactState(false, false);
}

TraintasticSensorNode::~TraintasticSensorNode()
{
    setSensor(nullptr);
}

bool TraintasticSensorNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractDeviatorNode::loadFromJSON(obj))
        return false;

    const QString sensorName = obj.value("sensor").toString();
    auto model = modeMgr()->modelForType(TraintasticSensorObj::Type);

    if(model)
        setSensor(static_cast<TraintasticSensorObj *>(model->getObjectByName(sensorName)));
    else
        setSensor(nullptr);

    setTargetState(obj.value("target_state").toInt(0));

    return true;
}

void TraintasticSensorNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);

    obj["sensor"] = mSensor ? mSensor->name() : QString();
    obj["target_state"] = mTargetState;
}

void TraintasticSensorNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty butProp;
    butProp.name = "sensor";
    butProp.prettyName = tr("Sensor");
    butProp.types = {TraintasticSensorObj::Type};
    result.append(butProp);
}

QString TraintasticSensorNode::nodeType() const
{
    return NodeType;
}

TraintasticSensorObj *TraintasticSensorNode::sensor() const
{
    return mSensor;
}

void TraintasticSensorNode::setSensor(TraintasticSensorObj *newSensor)
{
    if (mSensor == newSensor)
        return;

    if(mSensor)
    {
        mSensor->removeContactNode(this);
    }

    mSensor = newSensor;

    if(mSensor)
    {
        mSensor->addContactNode(this);
    }

    emit sensorChanged(mSensor);

    refreshContactState();
    modeMgr()->setFileEdited();
}

void TraintasticSensorNode::refreshContactState()
{
    if(!mSensor)
    {
        setContactState(false, false);
        return;
    }

    const bool straightContactOn = mSensor->state() == mTargetState;
    setContactState(!straightContactOn, straightContactOn);

    // Force button redraw
    emit deviatorStateChanged();
}

int TraintasticSensorNode::targetState() const
{
    return mTargetState;
}

void TraintasticSensorNode::setTargetState(int newTargetState)
{
    if(mTargetState == newTargetState)
        return;

    mTargetState = newTargetState;

    refreshContactState();
    emit shapeChanged();
    modeMgr()->setFileEdited();
}
