/**
 * src/objects/interfaces/leverinterface.cpp
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

#include "leverinterface.h"
#include "../abstractsimulationobject.h"

#include "../../circuits/nodes/levercontactnode.h"

#include <QJsonObject>

LeverInterface::LeverInterface(const EnumDesc &posDesc,
                               const LeverAngleDesc &angleDesc,
                               AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
    , mPositionDesc(posDesc)
    , mAngleDesc(angleDesc)
{

}

LeverInterface::~LeverInterface()
{
    auto contactNodes = mContactNodes;
    for(LeverContactNode *c : contactNodes)
    {
        c->setLever(nullptr);
    }

    stopSpringTimer();
}

QString LeverInterface::ifaceType()
{
    return IfaceType;
}

QVector<AbstractCircuitNode *> LeverInterface::nodes() const
{
    QVector<AbstractCircuitNode *> result;
    result.reserve(mContactNodes.size());
    for(auto item : mContactNodes)
        result.append(item);
    return result;
}

bool LeverInterface::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractObjectInterface::loadFromJSON(obj))
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

void LeverInterface::saveToJSON(QJsonObject &obj) const
{
    AbstractObjectInterface::saveToJSON(obj);

    obj["spring_return"] = hasSpringReturn();

    // Range
    obj["pos_min"] = mAbsoluteMin;
    obj["pos_max"] = mAbsoluteMax;

    obj["normal_position"] = normalPosition();
}

void LeverInterface::init()
{
    // Recalculate range
    setAbsoluteRange(mPositionDesc.minValue, mPositionDesc.maxValue);
    setNormalPosition(mPositionDesc.defaultValue);

    // Default to normal position
    setAngle(angleForPosition(normalPosition()));
}

int LeverInterface::angle() const
{
    return mAngle;
}

void LeverInterface::setAngle(int newAngle)
{
    const int MinAngleAbs= angleForPosition(mLockedMin);
    const int MaxAngleAbs = angleForPosition(mLockedMax);

    newAngle = qBound(MinAngleAbs, newAngle, MaxAngleAbs);

    if(mAngle == newAngle)
        return;

    mAngle = newAngle;
    setPosition(closestPosition(mAngle, true));

    emitChanged(AnglePropName, mAngle);
    emit mObject->stateChanged(mObject);
}

void LeverInterface::setAngleTrySnap(int newAngle)
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

int LeverInterface::position() const
{
    return mPosition;
}

void LeverInterface::setPosition(int newPosition)
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

        emitChanged(PositionPropName, mPosition);
        emit mObject->stateChanged(mObject);
    }
}

bool LeverInterface::hasSpringReturn() const
{
    return mHasSpringReturn;
}

void LeverInterface::setHasSpringReturn(bool newHasSpringReturn)
{
    if(!canChangeSpring())
        return;

    if(mHasSpringReturn == newHasSpringReturn)
        return;

    mHasSpringReturn = newHasSpringReturn;
    emit mObject->settingsChanged(mObject);

    if(mHasSpringReturn && !mIsPressed)
        startSpringTimer();
}

bool LeverInterface::isPressed() const
{
    return mIsPressed;
}

void LeverInterface::setPressed(bool newIsPressed)
{
    if(mIsPressed == newIsPressed)
        return;

    mIsPressed = newIsPressed;
    emitChanged(PressedPropName, mIsPressed);
    emit mObject->stateChanged(mObject);

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

int LeverInterface::closestPosition(int angle, bool allowMiddle) const
{
    for(int i = mLockedMin; i <= mLockedMax; i++)
    {
        if(isPositionMiddle(i))
        {
            const int prevPosAngle = angleForPosition(i - 1);
            const int nextPosAngle = angleForPosition(i + 1);

            if(angle <= prevPosAngle || angle >= nextPosAngle)
                continue;

            if(allowMiddle)
                return i; // Return middle position

            if((angle - prevPosAngle) < (nextPosAngle - angle))
                return (i - 1); // Closest to prev

            return (i + 1); // Closest to next
        }

        if(angle == angleForPosition(i))
            return i;
    }

    // Invalid
    return LeverPositionDesc::InvalidPosition;
}

void LeverInterface::setLockedRange(int newMin, int newMax)
{
    newMin = qMax(newMin, mAbsoluteMin);
    newMax = qMin(newMax, mAbsoluteMax);

    if(newMax < newMin)
        newMax = newMin;

    if(mLockedMin == newMin && mLockedMax == newMax)
        return;

    mLockedMin = newMin;
    mLockedMax = newMax;
}

void LeverInterface::checkPositionValidForLock()
{
    if(!isPositionValidForLock(position()))
    {
        // Current position is not valid anymore,
        // move lever to closest valid position.

        // Iterate from current position, up and down
        int diff = 1;
        while(true)
        {
            const int prevPos = position() - diff;
            const int nextPos = position() + diff;

            if(prevPos < absoluteMin() && nextPos > absoluteMax())
                break;

            if(prevPos >= absoluteMin() && isPositionValidForLock(prevPos))
            {
                setAngle(angleForPosition(prevPos));
                setPosition(prevPos);
                break;
            }

            if(nextPos <= absoluteMax() && isPositionValidForLock(nextPos))
            {
                setAngle(angleForPosition(nextPos));
                setPosition(nextPos);
                break;
            }

            diff++;
        }
    }
}

bool LeverInterface::timerEvent(const int timerId)
{
    if(timerId == springTimerId && springTimerId)
    {
        const int targetAngle = angleForPosition(normalPosition());

        if(qAbs(targetAngle - mAngle) <= SpringTimerAngleDelta)
        {
            // We reached target position
            stopSpringTimer();
            setAngle(targetAngle);
            return true;
        }

        int angleDelta = SpringTimerAngleDelta;
        if(targetAngle < mAngle)
            angleDelta = -angleDelta; // Go opposite direction

        const int newAngle = mAngle + angleDelta;
        setAngle(newAngle);

        if(angle() != newAngle)
            stopSpringTimer(); // Angle change was rejected

        return true;
    }

    return false;
}

void LeverInterface::stopSpringTimer()
{
    if(!springTimerId)
        return;

    mObject->killTimer(springTimerId);
    springTimerId = 0;
}

void LeverInterface::startSpringTimer()
{
    stopSpringTimer();

    // Update every 100ms for a semi-smooth animation
    springTimerId = mObject->startTimer(100);
}

int LeverInterface::absoluteMin() const
{
    return mAbsoluteMin;
}

int LeverInterface::absoluteMax() const
{
    return mAbsoluteMax;
}

void LeverInterface::setAbsoluteRange(int newMin, int newMax)
{
    if(!canChangeRange())
        return;

    newMin = qMax(newMin, 0);
    newMax = qMin(newMax, mPositionDesc.maxValue);

    if(newMax < newMin)
        newMax = newMin;

    const bool rangeChanged = (mAbsoluteMin != newMin) || (mAbsoluteMax != newMax);
    mAbsoluteMin = newMin;
    mAbsoluteMax = newMax;

    if(rangeChanged)
    {
        // Recalculate angle and normal position
        setAngle(mAngle);
        setNormalPosition(mNormalPosition);

        // Set fake locked range to allow every position
        // It will be recalculated by object if needed
        setLockedRange(absoluteMin(), absoluteMax());

        emitChanged(AbsoluteRangePropName, QVariant());
        emit mObject->settingsChanged(mObject);
    }
}

void LeverInterface::addContactNode(LeverContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));

    mContactNodes.append(c);

    emit mObject->nodesChanged();
}

void LeverInterface::removeContactNode(LeverContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->lever() == mObject);

    mContactNodes.removeOne(c);

    emit mObject->nodesChanged();
}

int LeverInterface::normalPosition() const
{
    return mNormalPosition;
}

void LeverInterface::setNormalPosition(int newNormalPosition)
{
    newNormalPosition = qBound(mAbsoluteMin,
                               newNormalPosition,
                               mAbsoluteMax);

    if(mNormalPosition == newNormalPosition)
        return;

    mNormalPosition = newNormalPosition;
    emit mObject->settingsChanged(mObject);
}

