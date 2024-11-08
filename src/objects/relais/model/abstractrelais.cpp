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

QString AbstractRelais::getTypeName(Type t)
{
    switch(t)
    {
    case Type::Normal:
        return tr("Normal");
    case Type::Polarized:
        return tr("Polarized");
    case Type::PolarizedInverted:
        return tr("Polarized Inverted");
    case Type::Stabilized:
        return tr("Stabilized");
    case Type::Combinator:
        return tr("Combinator");
    }

    return QString();
}

AbstractRelais::AbstractRelais(QObject *parent)
    : QObject{parent}
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

bool AbstractRelais::loadFromJSON(const QJsonObject &obj)
{
    setName(obj.value("name").toString());
    setUpSpeed(obj.value("speed_up").toDouble());
    setDownSpeed(obj.value("speed_down").toDouble());
    setNormallyUp(obj.value("normally_up").toBool());
    setType(Type(obj.value("relay_type").toInt(int(Type::Normal))));
    return true;
}

void AbstractRelais::saveToJSON(QJsonObject &obj) const
{
    obj["name"] = mName;
    obj["speed_up"] = mUpSpeed;
    obj["speed_down"] = mDownSpeed;
    obj["normally_up"] = normallyUp();
    obj["relay_type"] = int(type());
}

bool AbstractRelais::isStateIndependent(Type t)
{
    switch(t)
    {
    case AbstractRelais::Type::Stabilized:
    case AbstractRelais::Type::Combinator:
        return true;
    default:
        break;
    }

    return false;
}

QString AbstractRelais::name() const
{
    return mName;
}

void AbstractRelais::setName(const QString &newName)
{
    if (mName == newName)
        return;
    mName = newName;
    emit nameChanged(this, mName);
    emit settingsChanged(this);

    for(RelaisPowerNode *p : mPowerNodes)
    {
        p->setObjectName(mName);
    }

    for(RelaisContactNode *c : mContactNodes)
    {
        c->setObjectName(mName);
    }
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
    Q_ASSERT(!mPowerNodes.contains(p));

    mPowerNodes.append(p);
    p->setObjectName(mName);
}

void AbstractRelais::removePowerNode(RelaisPowerNode *p)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->relais() == this);

    mPowerNodes.removeOne(p);
}

void AbstractRelais::addContactNode(RelaisContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));

    mContactNodes.append(c);
    c->setObjectName(mName);
}

void AbstractRelais::removeContactNode(RelaisContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->relais() == this);

    mContactNodes.removeOne(c);
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

    emit powerChanged(this);
}

void AbstractRelais::powerNodeDeactivated(RelaisPowerNode *p, bool secondContact)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->relais() == this);

    const bool hadActiveUp = mActivePowerNodesUp > 0;
    const bool hadActiveDown = mActivePowerNodesDown > 0;

    if(stateIndependent() && secondContact)
    {
        Q_ASSERT(mActivePowerNodesDown > 0);
        mActivePowerNodesDown--;
    }
    else
    {
        Q_ASSERT(mActivePowerNodesUp > 0);
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

    emit powerChanged(this);
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

AbstractRelais::Type AbstractRelais::type() const
{
    return mType;
}

void AbstractRelais::setType(Type newType)
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
    emit stateChanged(this, mState);
}
