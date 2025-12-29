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

int LeverInterface::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    if(result)
    {
        for(auto item : mContactNodes)
            result->append(item);
    }

    return mContactNodes.size();
}

bool LeverInterface::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractObjectInterface::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    setHasSpringReturnMin(obj.value("spring_return_min").toBool());
    setHasSpringReturnMax(obj.value("spring_return_max").toBool());

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

    obj["spring_return_min"] = hasSpringReturnMin();
    obj["spring_return_max"] = hasSpringReturnMax();

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

    // Set explicitly in case angle does not trigger change
    setPosition(normalPosition());
}

int LeverInterface::angle() const
{
    return mAngle;
}

void LeverInterface::setAngle(int newAngle)
{
    if(canWarpAroundZero())
    {
        if(newAngle < 0)
            newAngle += 360;
        else if(newAngle >= 360)
            newAngle -= 360;
    }

    if(mLockedMin != LeverAngleDesc::InvalidPosition
            && mLockedMax != LeverAngleDesc::InvalidPosition)
    {
        // Clamp to allowed locked range
        int MinAngleAbs= angleForPosition(mLockedMin);
        int MaxAngleAbs = angleForPosition(mLockedMax);

        if(canWarpAroundZero())
        {
            if(MaxAngleAbs < MinAngleAbs)
            {
                MaxAngleAbs += 360;
                if(newAngle < MinAngleAbs)
                    newAngle += 360;
            }

            if(newAngle < MinAngleAbs || newAngle > MaxAngleAbs)
            {
                // Angle not in range, reject change
                return;
            }

            newAngle = newAngle % 360;
        }
        else
        {
            // For non-continuous levers we bound angle in range
            newAngle = qBound(MinAngleAbs, newAngle, MaxAngleAbs);
        }
    }
    else if(!canWarpAroundZero())
    {
        // Clamp to absolute range for non-continuous rotation levers
        const int MinAngleAbs= angleForPosition(mAbsoluteMin);
        int MaxAngleAbs = angleForPosition(mAbsoluteMax);
        if(MaxAngleAbs < MinAngleAbs)
            MaxAngleAbs += 360;

        newAngle = qBound(MinAngleAbs, newAngle, MaxAngleAbs);
    }

    if(mAngle == newAngle)
        return;

    mAngle = newAngle;

    // Now change position accordingly
    const int targetPosition = closestPosition(mAngle, true);
    setPosition(targetPosition);
    if(position() != targetPosition)
    {
        // Maybe locked range changed during position change
        // So we did not reach target position
        // Reset angle accordingly
        mAngle = angleForPosition(position());
        if(mAngle == LeverAngleDesc::MiddleAngle)
        {
            // Set it half way
            const int prevAngle = angleForPosition(position() - 1);
            const int nextAngle = angleForPosition(position() + 1);
            mAngle = (prevAngle + nextAngle) / 2;
        }
    }

    emitChanged(AnglePropName, mAngle);
    emit mObject->stateChanged(mObject);
}

void LeverInterface::setAngleTrySnap(int newAngle)
{
    if(newAngle < 0 && canWarpAroundZero())
        newAngle += 360;

    int pos = closestPosition(newAngle, true);
    if(pos != LeverAngleDesc::InvalidPosition && isPositionMiddle(pos))
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
    Q_ASSERT(newPosition >= 0);
    if(mPosition == newPosition)
        return;

    // Always ensure all positions are passed
    int increment = (newPosition > mPosition) ? +1 : -1;

    if(canWarpAroundZero())
    {
        // TODO: consider angles, but what about middle positions?
        const int normDiff = std::abs(newPosition - mPosition);
        const int totalDiff = mPositionDesc.maxValue - mPositionDesc.minValue + 1;
        const int reverseDiff = totalDiff - normDiff;
        if(reverseDiff < normDiff)
            increment = -increment;
    }

    int p = mPosition;
    while(p != newPosition)
    {
        p += increment;

        if(canWarpAroundZero())
        {
            if(p < 0)
                p = mPositionDesc.maxValue;
            else if(p > mPositionDesc.maxValue)
                p = mPositionDesc.minValue;

            if(mLockedMax != LeverAngleDesc::InvalidPosition && mLockedMin != LeverAngleDesc::InvalidPosition)
            {
                if(mLockedMax > mLockedMin)
                {
                    if(increment > 0 && p > mLockedMax)
                        break;
                    else if(increment < 0 && p < mLockedMin)
                        break;
                }
                else
                {
                    if(p < mLockedMin && p > mLockedMax)
                        break;
                }
            }
        }
        else
        {
            // Locked range can change during position change
            // Check at every step
            if(increment > 0 && p > mLockedMax && mLockedMax != LeverAngleDesc::InvalidPosition)
                break;
            else if(increment < 0 && p < mLockedMin && mLockedMin != LeverAngleDesc::InvalidPosition)
                break;
        }

        mPosition = p;

        emitChanged(PositionPropName, mPosition);
        emit mObject->stateChanged(mObject);
    }
}

void LeverInterface::setPositionDesc(const EnumDesc &desc_,
                                     const LeverAngleDesc &angleDesc_)
{
    // Reset lever conditions
    for(LeverContactNode *node : std::as_const(mContactNodes))
    {
        node->setConditionSet(LeverPositionConditionSet());
    }

    mPositionDesc = desc_;
    mAngleDesc = angleDesc_;

    // Re-init
    init();

    emitChanged(PosDescPropName, QVariant());
}

bool LeverInterface::hasSpringReturnMin() const
{
    return mHasSpringReturnMin;
}

void LeverInterface::setHasSpringReturnMin(bool newHasSpringReturn)
{
    if(!canChangeSpring())
        return;

    if(mHasSpringReturnMin == newHasSpringReturn)
        return;

    mHasSpringReturnMin = newHasSpringReturn;
    emit mObject->settingsChanged(mObject);

    if(mHasSpringReturnMin && !mIsPressed && mAngle < angleForPosition(mPositionDesc.defaultValue))
        startSpringTimer();
}

bool LeverInterface::hasSpringReturnMax() const
{
    return mHasSpringReturnMax;
}

void LeverInterface::setHasSpringReturnMax(bool newHasSpringReturn)
{
    if(!canChangeSpring())
        return;

    if(mHasSpringReturnMax == newHasSpringReturn)
        return;

    mHasSpringReturnMax = newHasSpringReturn;
    emit mObject->settingsChanged(mObject);

    if(mHasSpringReturnMax && !mIsPressed && mAngle > angleForPosition(mPositionDesc.defaultValue))
        startSpringTimer();
}

bool LeverInterface::isPressed() const
{
    return mIsPressed;
}

void LeverInterface::setPressed(bool newIsPressed, bool holdSpring)
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
    else if(mHasSpringReturnMin && mAngle < angleForPosition(mPositionDesc.defaultValue) && !holdSpring)
    {
        // When released, if lever has spring, go back to Normal
        startSpringTimer();
    }
    else if(mHasSpringReturnMax && mAngle > angleForPosition(mPositionDesc.defaultValue) && !holdSpring)
    {
        // When released, if lever has spring, go back to Normal
        startSpringTimer();
    }
}

int LeverInterface::positionForAngle_internal(int pos, int newAngle, bool allowMiddle) const
{
    if(isPositionMiddle(pos))
    {
        const int prevPosAngle = angleForPosition(pos - 1);
        int nextPosAngle = 0;

        if(canWarpAroundZero() && pos == mPositionDesc.maxValue)
        {
            // Do a full circle (360 degrees) and start from first position
            nextPosAngle = angleForPosition(0);
        }
        else
        {
            nextPosAngle = angleForPosition(pos + 1);
        }

        int adjustedAngle = newAngle;

        if(canWarpAroundZero() && prevPosAngle > nextPosAngle)
        {
            nextPosAngle += 360;
            if(adjustedAngle <= 180)
                adjustedAngle += 360;
        }

        if(adjustedAngle <= prevPosAngle || adjustedAngle >= nextPosAngle)
        {
            // Not in our range, check next position
            return LeverAngleDesc::InvalidPosition;
        }

        if(allowMiddle)
            return pos; // Return middle position

        if((adjustedAngle - prevPosAngle) < (nextPosAngle - adjustedAngle))
        {
            // Closest to prev
            const int prevPos = pos - 1;
            if(canWarpAroundZero() && prevPos < 0)
                return prevPos + mPositionDesc.maxValue + 1;
            return prevPos;
        }

        // Closest to next
        const int nextPos = pos + 1;
        if(canWarpAroundZero() && nextPos > mPositionDesc.maxValue)
            return nextPos - mPositionDesc.maxValue - 1;
        return nextPos;
    }

    if(newAngle == angleForPosition(pos))
        return pos;

    // Not our angle, check next position
    return LeverAngleDesc::InvalidPosition;
}

int LeverInterface::closestPosition(int newAngle, bool allowMiddle) const
{
    if(canWarpAroundZero())
    {
        if(newAngle < 0)
            newAngle += 360;
        else if(newAngle >= 360)
            newAngle -= 360;
    }

    int fromPos = mLockedMin;
    int toPos = mLockedMax;
    if(fromPos == LeverAngleDesc::InvalidPosition || toPos == LeverAngleDesc::InvalidPosition)
    {
        // Not locked
        fromPos = mAbsoluteMin;
        toPos = mAbsoluteMax;
    }

    if(fromPos <= toPos)
    {
        for(int i = fromPos; i <= toPos; i++)
        {
            int pos = positionForAngle_internal(i, newAngle, allowMiddle);
            if(pos != LeverAngleDesc::InvalidPosition)
                return pos;
        }
    }
    else
    {
        Q_ASSERT(canWarpAroundZero());
        for(int i = toPos; i <= mPositionDesc.maxValue; i++)
        {
            int pos = positionForAngle_internal(i, newAngle, allowMiddle);
            if(pos != LeverAngleDesc::InvalidPosition)
                return pos;
        }
        for(int i = mPositionDesc.minValue; i <= fromPos; i++)
        {
            int pos = positionForAngle_internal(i, newAngle, allowMiddle);
            if(pos != LeverAngleDesc::InvalidPosition)
                return pos;
        }
    }

    // Invalid
    return LeverPositionDesc::InvalidPosition;
}

void LeverInterface::setLockedRange(int newMin, int newMax)
{
    if(newMin == LeverAngleDesc::InvalidPosition
            || newMax == LeverAngleDesc::InvalidPosition)
    {
        // Remove locking
        newMin = newMax = LeverAngleDesc::InvalidPosition;
    }
    else
    {
        // Clamp locked range to absolute range
        newMin = qMax(newMin, mAbsoluteMin);
        newMax = qMin(newMax, mAbsoluteMax);

        if(newMax < newMin && !canWarpAroundZero())
            newMax = newMin;
    }

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
    if(timerId == springTimer.timerId())
    {
        const int targetAngle = angleForPosition(mPositionDesc.defaultValue);

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
    springTimer.stop();
}

void LeverInterface::startSpringTimer()
{
    stopSpringTimer();

    // Update every 100ms for a semi-smooth animation
    springTimer.start(std::chrono::milliseconds(100), mObject);
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

        // Unset locked range to allow every position
        // It will be recalculated by object if needed
        setLockedRange(LeverAngleDesc::InvalidPosition,
                       LeverAngleDesc::InvalidPosition);

        emitChanged(AbsoluteRangePropName, QVariant());
        emit mObject->settingsChanged(mObject);
    }
}

void LeverInterface::addContactNode(LeverContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));

    mContactNodes.append(c);

    emit mObject->nodesChanged(mObject);
}

void LeverInterface::removeContactNode(LeverContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->lever() == mObject);

    mContactNodes.removeOne(c);

    emit mObject->nodesChanged(mObject);
}

void LeverInterface::setCanWarpAroundZero(bool newCanWarpAroundZero)
{
    mCanWarpAroundZero = newCanWarpAroundZero;
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

