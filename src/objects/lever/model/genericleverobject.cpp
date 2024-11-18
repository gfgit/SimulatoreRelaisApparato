/**
 * src/objects/lever/model/genericleverobject.cpp
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

#include "genericleverobject.h"

#include "../../../circuits/nodes/levercontactnode.h"

#include <QTimerEvent>

#include <QJsonObject>


GenericLeverObject::GenericLeverObject(AbstractSimulationObjectModel *m, const LeverPositionDesc &desc)
    : AbstractSimulationObject{m}
    , mPositionDesc(desc)
{
    // Recalculate range
    setAbsoluteRange(0, mPositionDesc.maxPosition());

    // Default to normal position
    setNormalPosition(positionDesc().defaultPositionIdx);
    setAngle(angleForPosition(normalPosition()));
    setPosition(normalPosition());
}

GenericLeverObject::~GenericLeverObject()
{
    auto contactNodes = mContactNodes;
    for(LeverContactNode *c : contactNodes)
    {
        c->setLever(nullptr);
    }

    stopSpringTimer();
}

bool GenericLeverObject::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractSimulationObject::loadFromJSON(obj))
        return false;

    setHasSpringReturn(obj.value("spring_return").toBool());

    // Range
    int pos_min = obj.value("pos_min").toInt();
    int pos_max = obj.value("pos_max").toInt();
    setAbsoluteRange(pos_min, pos_max);

    // Load normal position if valid
    int normalPos = obj.value("normal_position").toInt(LeverPositionDesc::InvalidPosition);
    if(normalPos != LeverPositionDesc::InvalidPosition)
        setNormalPosition(normalPos);

    // Move lever to normal position
    setAngle(angleForPosition(normalPosition()));
    setPosition(normalPosition());

    return true;
}

void GenericLeverObject::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["spring_return"] = hasSpringReturn();

    // Range
    obj["pos_min"] = mAbsoluteMin;
    obj["pos_max"] = mAbsoluteMax;

    obj["normal_position"] = normalPosition();
}

QVector<AbstractCircuitNode *> GenericLeverObject::nodes() const
{
    QVector<AbstractCircuitNode *> result;
    result.reserve(mContactNodes.size());
    for(auto item : mContactNodes)
        result.append(item);
    return result;
}

int GenericLeverObject::angle() const
{
    return mAngle;
}

void GenericLeverObject::setAngle(int newAngle)
{
    const int MinAngleAbs= angleForPosition(mLockedMin);
    const int MaxAngleAbs = angleForPosition(mLockedMax);

    newAngle = qBound(MinAngleAbs, newAngle, MaxAngleAbs);

    if(mAngle == newAngle)
        return;

    mAngle = newAngle;
    setPosition(closestPosition(mAngle, true));

    emit angleChanged(this, mAngle);
    emit stateChanged(this);
}

void GenericLeverObject::setAngleTrySnap(int newAngle)
{
    int pos = closestPosition(newAngle, true);
    if(isPositionMiddle(pos))
    {
        // Try snap
        pos = snapTarget(newAngle);
        if(pos != LeverPositionDesc::InvalidPosition)
        {
            // We can snap
            newAngle = angleForPosition(pos);
        }
    }

    setAngle(newAngle);
}

int GenericLeverObject::position() const
{
    return mPosition;
}

void GenericLeverObject::setPosition(int newPosition)
{
    if(mPosition == newPosition)
        return;

    // Always ensure all positions are passed
    const int increment = (newPosition > mPosition) ? +1 : -1;

    int p = mPosition;
    while(p != newPosition)
    {
        p += increment;

        mPosition = p;
        emit positionChanged(this, mPosition);
        emit stateChanged(this);
    }

    recalculateLockedRange();
}

bool GenericLeverObject::hasSpringReturn() const
{
    return mHasSpringReturn;
}

void GenericLeverObject::setHasSpringReturn(bool newHasSpringReturn)
{
    if(mHasSpringReturn == newHasSpringReturn)
        return;

    mHasSpringReturn = newHasSpringReturn;
    emit settingsChanged(this);

    if(mHasSpringReturn && !mIsPressed)
        startSpringTimer();
}

bool GenericLeverObject::isPressed() const
{
    return mIsPressed;
}

void GenericLeverObject::setPressed(bool newIsPressed)
{
    if(mIsPressed == newIsPressed)
        return;

    mIsPressed = newIsPressed;
    emit pressedChanged(mIsPressed);
    emit stateChanged(this);

    if(mIsPressed)
    {
        // When lever is hold, spring cannot move lever
        stopSpringTimer();
    }
    else if(mHasSpringReturn)
    {
        // When released, if lever has spring, go back to Normal
        startSpringTimer();
    }
}

int GenericLeverObject::closestPosition(int angle, bool allowMiddle) const
{
    for(int i = mLockedMin; i <= mLockedMax; i++)
    {
        if(mPositionDesc.isMiddle(i))
        {
            const int prevPosAngle = mPositionDesc.angleFor(i - 1);
            const int nextPosAngle = mPositionDesc.angleFor(i + 1);

            if(angle <= prevPosAngle || angle >= nextPosAngle)
                continue;

            if(allowMiddle)
                return i; // Return middle position

            if((angle - prevPosAngle) < (nextPosAngle - angle))
                return (i - 1); // Closest to prev

            return (i + 1); // Closest to next
        }

        if(angle == mPositionDesc.angleFor(i))
            return i;
    }

    // Invalid
    return LeverPositionDesc::InvalidPosition;
}

void GenericLeverObject::recalculateLockedRange()
{
    // Default to no locking
    mLockedMin = mAbsoluteMin;
    mLockedMax = mAbsoluteMax;
}

void GenericLeverObject::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == springTimerId && springTimerId)
    {
        const int targetAngle = angleForPosition(normalPosition());

        if(qAbs(targetAngle - mAngle) <= SpringTimerAngleDelta)
        {
            // We reached target position
            stopSpringTimer();
            setAngle(targetAngle);
            return;
        }

        int angleDelta = SpringTimerAngleDelta;
        if(targetAngle < mAngle)
            angleDelta = -angleDelta; // Go opposite direction

        const int newAngle = mAngle + angleDelta;
        setAngle(newAngle);
        return;
    }

    QObject::timerEvent(e);
}

void GenericLeverObject::stopSpringTimer()
{
    if(!springTimerId)
        return;

    killTimer(springTimerId);
    springTimerId = 0;
}

void GenericLeverObject::startSpringTimer()
{
    stopSpringTimer();

    // Update every 100ms for a semi-smooth animation
    springTimerId = startTimer(100);
}

int GenericLeverObject::absoluteMin() const
{
    return mAbsoluteMin;
}

int GenericLeverObject::absoluteMax() const
{
    return mAbsoluteMax;
}

void GenericLeverObject::setAbsoluteRange(int newMin, int newMax)
{
    newMin = qMax(newMin, 0);
    newMax = qMin(newMax, mPositionDesc.maxPosition());

    if(newMax < newMin)
        newMax = newMin;

    const bool rangeChanged = (mAbsoluteMin != newMin) || (mAbsoluteMax != newMax);
    mAbsoluteMin = newMin;
    mAbsoluteMax = newMax;

    // Recalculate range, angle and normal position
    recalculateLockedRange();
    setAngle(mAngle);
    setNormalPosition(mNormalPosition);

    if(rangeChanged)
        emit settingsChanged(this);
}

void GenericLeverObject::addContactNode(LeverContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));

    mContactNodes.append(c);
    c->setObjectName(mName);
}

void GenericLeverObject::removeContactNode(LeverContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->lever() == this);

    mContactNodes.removeOne(c);
}

int GenericLeverObject::normalPosition() const
{
    return mNormalPosition;
}

void GenericLeverObject::setNormalPosition(int newNormalPosition)
{
    newNormalPosition = qBound(mAbsoluteMin,
                               newNormalPosition,
                               mAbsoluteMax);

    if(mNormalPosition == newNormalPosition)
        return;

    mNormalPosition = newNormalPosition;
    emit settingsChanged(this);
}

const LeverPositionDesc& GenericLeverObject::positionDesc() const
{
    return mPositionDesc;
}
