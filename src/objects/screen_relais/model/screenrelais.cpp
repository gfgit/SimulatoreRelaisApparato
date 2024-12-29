/**
 * src/objects/screen_relais/model/screenrelais.cpp
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

#include "screenrelais.h"

#include "../../../circuits/nodes/screenrelaiscontactnode.h"
#include "../../../circuits/nodes/screenrelaispowernode.h"

#include <QTimerEvent>

#include <QJsonObject>

#include <QRandomGenerator>

QString ScreenRelais::getScreenTypeName(ScreenType t)
{
    switch(t)
    {
    case ScreenType::CenteredScreen:
        return tr("Centered Screen");
    case ScreenType::DecenteredScreen:
        return tr("Decentered Screen");
    case ScreenType::NTypes:
        break;
    }

    return QString();
}

ScreenRelais::ScreenRelais(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{

}

ScreenRelais::~ScreenRelais()
{
    if(mPowerNode)
    {
        mPowerNode->setScreenRelais(nullptr);
    }

    auto contactNodes = mContactNodes;
    for(ScreenRelaisContactNode *c : contactNodes)
    {
        c->setScreenRelais(nullptr);
    }

    killTimer(mTimerId);
    mTimerId = 0;
}

QString ScreenRelais::getType() const
{
    return Type;
}

bool ScreenRelais::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    setScreenType(ScreenType(obj.value("screen_type").toInt(int(ScreenType::CenteredScreen))));

    return true;
}

void ScreenRelais::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["screen_type"] = int(screenType());
}

QVector<AbstractCircuitNode *> ScreenRelais::nodes() const
{
    QVector<AbstractCircuitNode *> result;
    result.reserve(mContactNodes.size() + 1);

    if(mPowerNode)
        result.append(mPowerNode);

    for(auto item : mContactNodes)
        result.append(item);

    return result;
}

void ScreenRelais::timerEvent(QTimerEvent *e)
{
    if(mTimerId && e->timerId() == mTimerId)
    {
        if(qFuzzyCompare(mTargetPosition, mPosition))
        {
            killTimer(mTimerId);
            mTimerId = 0;
        }
        else
        {
            const bool goingToCenter = qFuzzyIsNull(mTargetPosition);

            double newPosition = mPosition;
            if(mTargetPosition > newPosition)
            {
                newPosition += 0.15;
            }
            else
            {
                newPosition -= 0.15;
            }

            if(goingToCenter && qAbs(mPosition) < 0.2)
                newPosition = 0.0; // TODO: oscillate a bit

            setPosition(newPosition);
        }
        return;
    }

    QObject::timerEvent(e);
}

void ScreenRelais::setPowerState(PowerState newState)
{
    if(!mPowerNode)
        newState = PowerState::None;

    if(mState == newState)
        return;

    mState = newState;

    mTargetPosition = 0.0;

    switch (screenType())
    {
    case ScreenType::CenteredScreen:
    {
        switch (mState)
        {
        case PowerState::None:
            mTargetPosition = 0;
            break;
        case PowerState::Direct:
            mTargetPosition = 1;
            break;
        case PowerState::Reversed:
            mTargetPosition = -1;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    if(!mTimerId)
        mTimerId = startTimer(100);
}

void ScreenRelais::setPosition(double newPosition)
{
    newPosition = qBound(-1.0, newPosition, 1.0);

    if(qFuzzyCompare(mPosition, newPosition))
        return;

    mPosition = newPosition;

    if(qFuzzyCompare(mTargetPosition, mPosition))
    {
        killTimer(mTimerId);
        mTimerId = 0;
    }

    // Update contact state
    const auto stateA = ScreenRelaisContactNode::ContactState(getContactStateA());
    const auto stateB = ScreenRelaisContactNode::ContactState(getContactStateB());
    for(ScreenRelaisContactNode *node : std::as_const(mContactNodes))
    {
        node->setState(node->isContactA() ? stateA : stateB);
    }

    emit stateChanged(this);
}

ScreenRelais::ScreenType ScreenRelais::screenType() const
{
    return mType;
}

void ScreenRelais::setScreenType(ScreenType newType)
{
    if(mType == newType)
        return;

    mType = newType;
    emit settingsChanged(this);
    emit typeChanged(this, mType);
}

ScreenRelais::ContactState ScreenRelais::getContactStateA() const
{
    switch (screenType())
    {
    case ScreenType::CenteredScreen:
    {
        if(mPosition < -0.5)
            return ContactState::Reversed;
        if(mPosition < -0.2)
            return ContactState::Middle;

        return ContactState::Straight;
    }
    default:
        break;
    }

    return ContactState::Middle;
}

ScreenRelais::ContactState ScreenRelais::getContactStateB() const
{
    switch (screenType())
    {
    case ScreenType::CenteredScreen:
    {
        if(mPosition > 0.5)
            return ContactState::Reversed;
        if(mPosition > 0.2)
            return ContactState::Middle;

        return ContactState::Straight;
    }
    default:
        break;
    }

    return ContactState::Middle;
}

void ScreenRelais::setPowerNode(ScreenRelaisPowerNode *node)
{
    if(mPowerNode == node)
        return;

    mPowerNode = node;

    if(!mPowerNode)
    {
        setPowerState(PowerState::None);
    }

    emit nodesChanged();
    emit settingsChanged(this);
}

void ScreenRelais::addContactNode(ScreenRelaisContactNode *node)
{
    Q_ASSERT_X(!mContactNodes.contains(node),
               "addContactNode", "already added");

    mContactNodes.append(node);

    const ContactState s = node->isContactA() ? getContactStateA() : getContactStateB();
    node->setState(ScreenRelaisContactNode::ContactState(s));

    emit nodesChanged();
    emit settingsChanged(this);
}

void ScreenRelais::removeContactNode(ScreenRelaisContactNode *node)
{
    Q_ASSERT_X(mContactNodes.contains(node),
               "removeContactNode", "not registered");
    Q_ASSERT_X(node->screenRelais() == this,
               "removeContactNode", "relay does not match");

    mContactNodes.removeOne(node);

    emit nodesChanged();
}
