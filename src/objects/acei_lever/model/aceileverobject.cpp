/**
 * src/objects/acei_lever/model/aceileverobject.cpp
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

#include "aceileverobject.h"

#include <QTimerEvent>

ACEILeverObject::ACEILeverObject(QObject *parent)
    : QObject{parent}
{

}

QString ACEILeverObject::name() const
{
    return mName;
}

void ACEILeverObject::setName(const QString &newName)
{
    if (mName == newName)
        return;
    mName = newName;

    emit nameChanged(this, mName);
}

int ACEILeverObject::angle() const
{
    return mAngle;
}

void ACEILeverObject::setAngle(int newAngle)
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

void ACEILeverObject::setAngleTrySnap(int newAngle)
{
    ACEILeverPosition pos = closestPosition(newAngle, true);
    if(isPositionMiddle(pos))
    {
        // Try snap
        pos = snapTarget(newAngle);
        if(pos != ACEILeverPosition::NPositions)
        {
            // We can snap
            newAngle = angleForPosition(pos);
        }
    }

    setAngle(newAngle);
}

ACEILeverPosition ACEILeverObject::position() const
{
    return mPosition;
}

void ACEILeverObject::setPosition(ACEILeverPosition newPosition)
{
    if(mPosition == newPosition)
        return;

    // Always ensure all positions are passed
    const int increment = (newPosition > mPosition) ? +1 : -1;

    int p = int(mPosition);
    while(p != int(newPosition))
    {
        p += increment;

        mPosition = ACEILeverPosition(p);
        emit positionChanged(this, mPosition);
    }
}

bool ACEILeverObject::hasSpringReturn() const
{
    return mHasSpringReturn;
}

void ACEILeverObject::setHasSpringReturn(bool newHasSpringReturn)
{
    if(mHasSpringReturn == newHasSpringReturn)
        return;

    mHasSpringReturn = newHasSpringReturn;
    emit changed(this);

    if(mHasSpringReturn && !mIsPressed)
        startSpringTimer();
}

bool ACEILeverObject::isPressed() const
{
    return mIsPressed;
}

void ACEILeverObject::setPressed(bool newIsPressed)
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

void ACEILeverObject::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == springTimerId && springTimerId)
    {
        constexpr int targetAngle = angleForPosition(ACEILeverPosition::Normal);

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

void ACEILeverObject::stopSpringTimer()
{
    if(!springTimerId)
        return;

    killTimer(springTimerId);
    springTimerId = 0;
}

void ACEILeverObject::startSpringTimer()
{
    stopSpringTimer();

    // Update every 100ms for a semi-smooth animation
    springTimerId = startTimer(100);
}

ACEILeverPosition ACEILeverObject::absoluteMax() const
{
    return mAbsoluteMax;
}

ACEILeverPosition ACEILeverObject::absoluteMin() const
{
    return mAbsoluteMin;
}

void ACEILeverObject::setAbsoluteRange(ACEILeverPosition newMin, ACEILeverPosition newMax)
{
    if(newMax < newMin)
        newMax = newMin;

    const bool rangeChanged = (mAbsoluteMin != newMin) || (mAbsoluteMax != newMax);
    mAbsoluteMin = newMin;
    mAbsoluteMax = newMax;

    if(rangeChanged)
        emit changed(this);

    // Recalculate angle
    setAngle(mAngle);
}
