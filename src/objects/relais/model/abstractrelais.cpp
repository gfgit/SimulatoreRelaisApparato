/**
 * src/objects/relais/model/abstractrelais.cpp
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

#include "abstractrelais.h"

#include "../../../circuits/nodes/relaiscontactnode.h"
#include "../../../circuits/nodes/relaispowernode.h"

#include <QTimerEvent>

#include <QJsonObject>

QString AbstractRelais::getRelaisTypeName(RelaisType t)
{
    switch(t)
    {
    case RelaisType::Normal:
        return tr("Normal");
    case RelaisType::Polarized:
        return tr("Polarized");
    case RelaisType::PolarizedInverted:
        return tr("Polarized Inverted");
    case RelaisType::Stabilized:
        return tr("Stabilized");
    case RelaisType::Combinator:
        return tr("Combinator");
    case RelaisType::NTypes:
        break;
    }

    return QString();
}

AbstractRelais::AbstractRelais(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{

}

AbstractRelais::~AbstractRelais()
{
    auto powerNodes = mPowerNodes;
    for(RelaisPowerNode *p : powerNodes)
    {
        p->setRelais(nullptr);
    }

    auto contactNodes = mContactNodes;
    for(RelaisContactNode *c : contactNodes)
    {
        c->setRelais(nullptr);
    }

    killTimer(mTimerId);
    mTimerId = 0;
}

QString AbstractRelais::getType() const
{
    return Type;
}

bool AbstractRelais::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    setUpSpeed(obj.value("speed_up").toDouble());
    setDownSpeed(obj.value("speed_down").toDouble());
    setNormallyUp(obj.value("normally_up").toBool());
    setRelaisType(RelaisType(obj.value("relay_type").toInt(int(RelaisType::Normal))));
    return true;
}

void AbstractRelais::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["speed_up"] = mUpSpeed;
    obj["speed_down"] = mDownSpeed;
    obj["normally_up"] = normallyUp();
    obj["relay_type"] = int(relaisType());
}

QVector<AbstractCircuitNode *> AbstractRelais::nodes() const
{
    QVector<AbstractCircuitNode *> result;
    result.reserve(mPowerNodes.size() + mContactNodes.size());
    for(auto item : mPowerNodes)
        result.append(item);
    for(auto item : mContactNodes)
        result.append(item);
    return result;
}

bool AbstractRelais::isStateIndependent(RelaisType t)
{
    switch(t)
    {
    case AbstractRelais::RelaisType::Stabilized:
    case AbstractRelais::RelaisType::Combinator:
        return true;
    default:
        break;
    }

    return false;
}

double AbstractRelais::upSpeed() const
{
    return mUpSpeed;
}

void AbstractRelais::setUpSpeed(double newUpSpeed)
{
    mUpSpeed = newUpSpeed;
}

double AbstractRelais::downSpeed() const
{
    return mDownSpeed;
}

void AbstractRelais::setDownSpeed(double newDownSpeed)
{
    mDownSpeed = newDownSpeed;
}

void AbstractRelais::addPowerNode(RelaisPowerNode *p)
{
    Q_ASSERT_X(!mPowerNodes.contains(p),
               "addPowerNode", "already added");

    mPowerNodes.append(p);

    emit nodesChanged();
}

void AbstractRelais::removePowerNode(RelaisPowerNode *p)
{
    Q_ASSERT_X(mPowerNodes.contains(p),
               "removePowerNode", "not registered");
    Q_ASSERT_X(p->relais() == this,
               "removePowerNode", "relay does not match");

    mPowerNodes.removeOne(p);

    emit nodesChanged();
}

void AbstractRelais::addContactNode(RelaisContactNode *c)
{
    Q_ASSERT_X(!mContactNodes.contains(c),
               "addContactNode", "already added");

    mContactNodes.append(c);

    emit nodesChanged();
}

void AbstractRelais::removeContactNode(RelaisContactNode *c)
{
    Q_ASSERT_X(mContactNodes.contains(c),
               "removeContactNode", "not registered");
    Q_ASSERT_X(c->relais() == this,
               "removeContactNode", "relay does not match");

    mContactNodes.removeOne(c);

    emit nodesChanged();
}

void AbstractRelais::timerEvent(QTimerEvent *e)
{
    if(mTimerId && e->timerId() == mTimerId)
    {
        double newPosition = mPosition;
        if(mInternalState == State::GoingUp)
            newPosition += mUpSpeed;
        else if(mInternalState == State::GoingDown)
            newPosition -= mDownSpeed;
        else
        {
            killTimer(mTimerId);
            mTimerId = 0;
        }

        if((newPosition < 0.0) || (newPosition > 1.0))
        {
            mInternalState = mState;
            killTimer(mTimerId);
            mTimerId = 0;
        }

        setPosition(newPosition);

        return;
    }

    QObject::timerEvent(e);
}

void AbstractRelais::powerNodeActivated(RelaisPowerNode *p, bool secondContact)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->relais() == this);

    const bool hadActiveUp = mActivePowerNodesUp > 0;
    const bool hadActiveDown = mActivePowerNodesDown > 0;

    if(stateIndependent() && secondContact)
    {
        mActivePowerNodesDown++;
    }
    else
    {
        mActivePowerNodesUp++;
    }

    if(stateIndependent())
    {
        if(mActivePowerNodesDown == 0)
        {
            if((hadActiveDown || !hadActiveUp) && mActivePowerNodesUp > 0)
            {
                startMove(true);
            }
        }
        else if(!hadActiveDown)
        {
            // Down circuit wins always if present
            startMove(false);
        }
    }
    else if(mActivePowerNodesUp == 1)
    {
        // Begin powering normal relais
        startMove(true);
    }

    emit stateChanged(this);
}

void AbstractRelais::powerNodeDeactivated(RelaisPowerNode *p, bool secondContact)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->relais() == this);

    const bool hadActiveUp = mActivePowerNodesUp > 0;
    const bool hadActiveDown = mActivePowerNodesDown > 0;

    if(stateIndependent() && secondContact)
    {
        Q_ASSERT_X(mActivePowerNodesDown > 0,
                   "powerNodeDeactivated",
                   "none active down");
        mActivePowerNodesDown--;
    }
    else
    {
        Q_ASSERT_X(mActivePowerNodesUp > 0,
                   "powerNodeDeactivated",
                   "none active up");
        mActivePowerNodesUp--;
    }

    if(stateIndependent())
    {
        if(mActivePowerNodesDown == 0)
        {
            if((hadActiveDown || !hadActiveUp) && mActivePowerNodesUp > 0)
            {
                startMove(true);
            }
        }
        else if(!hadActiveDown)
        {
            // Down circuit wins always if present
            startMove(false);
        }
    }
    else if(mActivePowerNodesUp == 0)
    {
        // End powering normal relais
        startMove(false);
    }

    emit stateChanged(this);
}

void AbstractRelais::setPosition(double newPosition)
{
    newPosition = qBound(0.0, newPosition, 1.0);

    if(qFuzzyCompare(mPosition, newPosition))
        return;

    const bool up = newPosition > mPosition;

    mPosition = newPosition;

    if(mPosition < 0.1)
        setState(State::Down);
    else if(mPosition > 0.9)
        setState(State::Up);
    else if(up)
        setState(State::GoingUp);
    else
        setState(State::GoingDown);
}

void AbstractRelais::startMove(bool up)
{
    mInternalState = up ? State::GoingUp : State::GoingDown;
    killTimer(mTimerId);
    mTimerId = startTimer(250);
}

AbstractRelais::RelaisType AbstractRelais::relaisType() const
{
    return mType;
}

void AbstractRelais::setRelaisType(RelaisType newType)
{
    if(mType == newType)
        return;

    mType = newType;
    emit settingsChanged(this);
    emit typeChanged(this, mType);
}

bool AbstractRelais::normallyUp() const
{
    return mNormallyUp;
}

void AbstractRelais::setNormallyUp(bool newNormallyUp)
{
    if(mNormallyUp == newNormallyUp)
        return;

    mNormallyUp = newNormallyUp;
    emit settingsChanged(this);

    // Update nodes
    for(RelaisPowerNode *p : mPowerNodes)
    {
        emit p->shapeChanged();
    }

    for(RelaisContactNode *c : mContactNodes)
    {
        emit c->shapeChanged();
    }
}

AbstractRelais::State AbstractRelais::state() const
{
    return mState;
}

void AbstractRelais::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;
    emit stateChanged(this);
}
