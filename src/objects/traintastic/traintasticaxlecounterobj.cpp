/**
 * src/objects/traintastic/traintasticaxlecounterobj.cpp
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

#include "traintasticaxlecounterobj.h"
#include "../abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"
#include "../../network/traintastic-simulator/traintasticsimmanager.h"

#include "../../circuits/nodes/traintasticaxlecounternode.h"

#include <QTimerEvent>

#include <QJsonObject>

TraintasticAxleCounterObj::TraintasticAxleCounterObj(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject(m)
{
    mSensors[1].invertCount = true;
}

TraintasticAxleCounterObj::~TraintasticAxleCounterObj()
{
    setChannel(InvalidChannel, true);
    setChannel(InvalidChannel, false);
    setAddress(InvalidAddress, true);
    setAddress(InvalidAddress, false);
    setContactNode(nullptr);
}

QString TraintasticAxleCounterObj::getType() const
{
    return Type;
}

bool TraintasticAxleCounterObj::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true;

    setChannel(obj.value("channel_0").toInt(InvalidChannel), true);
    setAddress(obj.value("address_0").toInt(InvalidAddress), true);
    setInvertCount(obj.value("invert_0").toBool(false), true);

    setChannel(obj.value("channel_1").toInt(InvalidChannel), false);
    setAddress(obj.value("address_1").toInt(InvalidAddress), false);
    setInvertCount(obj.value("invert_1").toBool(true), false);
    return true;
}

void TraintasticAxleCounterObj::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["channel_0"] = mSensors[0].channel;
    obj["address_0"] = mSensors[0].address;
    obj["invert_0"] = mSensors[0].invertCount;

    obj["channel_1"] = mSensors[1].channel;
    obj["address_1"] = mSensors[1].address;
    obj["invert_1"] = mSensors[1].invertCount;
}

int TraintasticAxleCounterObj::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int nodesCount = AbstractSimulationObject::getReferencingNodes(result);

    if(mContactNode)
    {
        nodesCount++;
        if(result)
            result->append(mContactNode);
    }

    return nodesCount;
}

void TraintasticAxleCounterObj::setChannel(int newChannel, bool first)
{
    Sensor &sensor = mSensors[first ? 0 : 1];
    if(sensor.channel == newChannel)
        return;

    sensor.channel = newChannel;
    emit settingsChanged(this);
}

void TraintasticAxleCounterObj::setAddress(int newAddress, bool first)
{
    Sensor &sensor = mSensors[first ? 0 : 1];
    if(sensor.address == newAddress)
        return;

    sensor.address = newAddress;
    emit settingsChanged(this);
}

void TraintasticAxleCounterObj::setInvertCount(bool invert, bool first)
{
    Sensor &sensor = mSensors[first ? 0 : 1];
    if(sensor.invertCount == invert)
        return;

    sensor.invertCount = invert;
    emit settingsChanged(this);
}

void TraintasticAxleCounterObj::axleCounterEvent(int32_t axleCountDiff, bool firstSensor)
{
    if(mState == State::OccupiedAtStart || isResetting() || axleCountDiff == 0)
        return;

    const bool invert = mSensors[firstSensor ? 0 : 1].invertCount;
    if(invert)
        axleCountDiff = -axleCountDiff;

    mAxleCount += axleCountDiff;
    setState(mAxleCount == 0 ? State::Free : State::Occupied);

    if(mContactNode)
    {
        mContactNode->updateState(false);
    }

    emit stateChanged(this);
}

QString TraintasticAxleCounterObj::getStateName() const
{
    switch (state())
    {
    case State::OccupiedAtStart:
        return tr("Occupied at start");
    case State::ResetPre:
        return tr("Reset (pre)");
    case State::Reset:
        return tr("Reset");
    case State::ResetPost:
        return tr("Reset (post)");
    case State::Free:
        return tr("Free");
    case State::Occupied:
        return tr("Occupied");
    default:
        break;
    }

    return QString();
}

void TraintasticAxleCounterObj::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mResetTimer.timerId())
    {
        mResetTimer.stop();
        if(mState == State::ResetPre)
            setState(State::Reset);
        else if(mState == State::Reset)
            setState(State::ResetPost);
        return;
    }

    AbstractSimulationObject::timerEvent(e);
}

void TraintasticAxleCounterObj::setState(State newState)
{
    if(newState == mState)
        return;

    mState = newState;
    if(mState == State::OccupiedAtStart || isResetting())
        mAxleCount = 0;

    if(mState == State::ResetPre)
        mResetTimer.start(std::chrono::milliseconds(3000), Qt::PreciseTimer, this);
    else if(mState == State::Reset)
        mResetTimer.start(std::chrono::milliseconds(10000), Qt::PreciseTimer, this);

    emit stateChanged(this);

    if(mContactNode)
    {
        mContactNode->updateState(true);
    }
}

void TraintasticAxleCounterObj::setContactNode(TraintasticAxleCounterNode *c)
{
    if(mContactNode == c)
        return;

    if(mContactNode)
    {
        TraintasticAxleCounterNode *oldContact = mContactNode;
        mContactNode = nullptr;
        oldContact->setAxleCounter(nullptr);
    }
    else
    {
        setHasPower(false);
        triggerReset(false);
    }

    mContactNode = c;
    emit settingsChanged(this);
    emit nodesChanged(this);
}

void TraintasticAxleCounterObj::triggerReset(bool val)
{
    if(val && !isResetting())
    {
        setState(State::ResetPre);
    }
    else if(!val && isResetting())
    {
        if(mHasPower && mState == State::Reset)
            setState(mAxleCount == 0 ? State::Free : State::Occupied);
        else
            setState(State::OccupiedAtStart);
    }
}

void TraintasticAxleCounterObj::setHasPower(bool val)
{
    if(mHasPower == val)
        return;

    mHasPower = val;

    if(!isResetting())
    {
        // When power is cut, unit resets itself to occupied
        // Regardless of current axle count
        setState(State::OccupiedAtStart);
    }
}
