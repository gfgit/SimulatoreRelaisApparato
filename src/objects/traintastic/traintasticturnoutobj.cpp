/**
 * src/objects/traintastic/traintasticturnoutobj.cpp
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

#include "traintasticturnoutobj.h"

#include "../../circuits/nodes/traintasticturnoutnode.h"

#include "../abstractsimulationobjectmodel.h"
#include "../../views/modemanager.h"

#include "../../network/traintastic-simulator/traintasticsimmanager.h"

#include <QTimerEvent>

#include <QJsonObject>

TraintasticTurnoutObj::TraintasticTurnoutObj(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject(m)
{
    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->addTurnout(this);
}

TraintasticTurnoutObj::~TraintasticTurnoutObj()
{
    setNode(nullptr);

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->removeTurnout(this);
}

QString TraintasticTurnoutObj::getType() const
{
    return Type;
}

bool TraintasticTurnoutObj::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    setInitialState(State(obj.value("init_state").toInt(State::Closed)));
    setTotalTimeMillis(obj.value("total_ms").toInt(8000));

    setChannel(obj.value("channel").toInt(0));
    setAddress(obj.value("address").toInt(InvalidAddress));

    switch (initialState())
    {
    case State::Closed:
        mPosition = 0;
        break;
    case State::Thrown:
        mPosition = 1;
        break;
    default:
        setInitialState(State::Unknown);
        mPosition = 0.5;
        break;
    }

    setState(initialState());

    return true;
}

void TraintasticTurnoutObj::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["channel"] = mChannel;
    obj["address"] = mAddress;
    obj["init_state"] = mInitialState;
    obj["total_ms"] = mTotalTimeMillis;
}

int TraintasticTurnoutObj::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    if(!mNode)
        return 0;

    if(result)
        result->append(mNode);
    return 1;
}

void TraintasticTurnoutObj::setChannel(int newChannel)
{
    if(mChannel == newChannel)
        return;

    mChannel = newChannel;
    emit settingsChanged(this);
}

void TraintasticTurnoutObj::setAddress(int newAddress)
{
    if(mAddress == newAddress)
        return;

    mAddress = newAddress;
    emit settingsChanged(this);
}

void TraintasticTurnoutObj::setInitialState(State newInitialState)
{
    if(mInitialState == newInitialState)
        return;

    mInitialState = newInitialState;
    emit settingsChanged(this);
}

void TraintasticTurnoutObj::setState(State newState)
{
    if(mState == newState)
        return;

    mState = newState;
    emit stateChanged(this);

    TraintasticSimManager *mgr = model()->modeMgr()->getTraitasticSimMgr();
    mgr->setTurnoutState(mChannel, mAddress, int(state()));
}

int TraintasticTurnoutObj::totalTimeMillis() const
{
    return mTotalTimeMillis;
}

void TraintasticTurnoutObj::setTotalTimeMillis(int newTotalTimeMillis)
{
    if(mTotalTimeMillis == newTotalTimeMillis)
        return;

    mTotalTimeMillis = newTotalTimeMillis;
    emit settingsChanged(this);
}

void TraintasticTurnoutObj::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == mTimer.timerId())
    {
        const double delta = double(TickMillis) / mTotalTimeMillis;
        const double newPos = mPosition + delta * (isGoingUp ? 1 : -1);
        setPosition(newPos);
        return;
    }

    AbstractSimulationObject::timerEvent(ev);
}

void TraintasticTurnoutObj::setNode(TraintasticTurnoutNode *node)
{
    if(mNode == node)
        return;

    if(mNode)
    {
        TraintasticTurnoutNode *oldNode = mNode;
        mNode = nullptr;
        oldNode->setTurnout(nullptr);
    }

    mNode = node;
    setActive(false, false);

    emit settingsChanged(this);
}

void TraintasticTurnoutObj::setActive(bool val, bool up)
{
    if(val == isActive() && (up == isGoingUp || !isActive()))
        return;

    if(val)
    {
        mTimer.start(TickMillis, Qt::CoarseTimer, this);
        isGoingUp = up;
    }
    else
    {
        mTimer.stop();

        // Reset turnout to end position
        if(mPosition <= ClosetThreshold)
            setPosition(0);
        else if(mPosition >= ThrownThreshold)
            setPosition(1);
    }

    emit stateChanged(this);
}

void TraintasticTurnoutObj::setPosition(double newPos)
{
    newPos = std::clamp(newPos, 0.0, 1.0);

    if(mPosition == newPos)
        return;

    mPosition = newPos;
    emit stateChanged(this);

    if(mPosition <= ClosetThreshold)
        setState(State::Closed);
    else if(mPosition >= ThrownThreshold)
        setState(State::Thrown);
    else
        setState(State::Unknown);
}
