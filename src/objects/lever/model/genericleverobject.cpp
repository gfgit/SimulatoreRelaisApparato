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


GenericLeverObject::GenericLeverObject(const LeverPositionDesc &desc, QObject *parent)
    : QObject{parent}
    , mPositionDesc(desc)
{
    // Recalculate range
    setAbsoluteRange(0, mPositionDesc.maxPosition());

    // Default to normal position
    setInitialPosition(positionDesc().normalPositionIdx);
    setAngle(angleForPosition(positionDesc().normalPositionIdx));
    setPosition(positionDesc().normalPositionIdx);
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
    setName(obj.value("name").toString());
    setHasSpringReturn(obj.value("spring_return").toBool());

    // Range
    int pos_min = obj.value("pos_min").toInt();
    int pos_max = obj.value("pos_max").toInt();
    setAbsoluteRange(pos_min, pos_max);

    // Load initial position if valid
    int initPos = obj.value("initial_position").toInt(LeverPositionDesc::InvalidPosition);
    if(initPos != LeverPositionDesc::InvalidPosition)
        setInitialPosition(initPos);

    // Move lever to initial position
    setAngle(angleForPosition(initialPosition()));
    setPosition(initialPosition());

    return true;
}

void GenericLeverObject::saveToJSON(QJsonObject &obj) const
{
    obj["name"] = mName;
    obj["spring_return"] = hasSpringReturn();

    // Range
    obj["pos_min"] = mAbsoluteMin;
    obj["pos_max"] = mAbsoluteMax;

    obj["initial_position"] = initialPosition();
}

QString GenericLeverObject::name() const
{
    return mName;
}

void GenericLeverObject::setName(const QString &newName)
{
    if (mName == newName)
        return;
    mName = newName;
    emit nameChanged(this, mName);

    for(LeverContactNode *c : mContactNodes)
    {
        c->setObjectName(mName);
    }
}

int GenericLeverObject::angle() const
{
    return mAngle;
}

void GenericLeverObject::setAngle(int newAngle)
{
    const int MinAngleAbs= angleForPosition(mAbsoluteMin);
    const int MaxAngleAbs = angleForPosition(mAbsoluteMax);

    newAngle = qBound(MinAngleAbs, newAngle, MaxAngleAbs);

    if(mAngle == newAngle)
        return;

    mAngle = newAngle;
    setPosition(closestPosition(mAngle, true));

    emit angleChanged(this, mAngle);
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
    }
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
    emit changed(this);

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
    for(int i = mAbsoluteMin; i <= mAbsoluteMax; i++)
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

void GenericLeverObject::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == springTimerId && springTimerId)
    {
        const int targetAngle = angleForPosition(mPositionDesc.normalPositionIdx);

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


    if(rangeChanged)
        emit changed(this);

    // Recalculate angle and initial position
    setAngle(mAngle);
    setInitialPosition(mInitialPosition);
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

int GenericLeverObject::initialPosition() const
{
    return mInitialPosition;
}

void GenericLeverObject::setInitialPosition(int newInitialPosition)
{
    newInitialPosition = qBound(mAbsoluteMin,
                                newInitialPosition,
                                mAbsoluteMax);

    if(mInitialPosition == newInitialPosition)
        return;

    mInitialPosition = newInitialPosition;
    emit changed(this);
}

const LeverPositionDesc& GenericLeverObject::positionDesc() const
{
    return mPositionDesc;
}
