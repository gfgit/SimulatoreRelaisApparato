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

#include "../../../circuits/nodes/levercontactnode.h"

#include "../../../enums/aceileverposition.h"

#include <QTimerEvent>

#include <QJsonObject>

static QString ACEILeverPosition_translate(const char *nameId)
{
    return ACEILeverObject::tr(nameId);
}

static const LeverPositionDesc::Item aceiLeverItems[] =
{
    {-90, QT_TRANSLATE_NOOP("ACEILeverObject", "Left")},
    {}, // Middle1
    {0,   QT_TRANSLATE_NOOP("ACEILeverObject", "Normal")},
    {}, // Middle2
    {+90, QT_TRANSLATE_NOOP("ACEILeverObject", "Right")}
};

static const LeverPositionDesc aceiLeverDesc(aceiLeverItems,
                                             int(ACEILeverPosition::Normal),
                                             &ACEILeverPosition_translate);

ACEILeverObject::ACEILeverObject(QObject *parent)
    : ACEILeverObject(aceiLeverDesc, parent)
{

}

ACEILeverObject::ACEILeverObject(const LeverPositionDesc &desc, QObject *parent)
    : QObject{parent}
    , mPositionDesc(aceiLeverDesc)
{
    // Recalculate range
    setAbsoluteRange(0, mPositionDesc.maxPosition());
}

ACEILeverObject::~ACEILeverObject()
{
    auto contactNodes = mContactNodes;
    for(LeverContactNode *c : contactNodes)
    {
        c->setLever(nullptr);
    }

    stopSpringTimer();
}

bool ACEILeverObject::loadFromJSON(const QJsonObject &obj)
{
    setName(obj.value("name").toString());
    setHasSpringReturn(obj.value("spring_return").toBool());

    // Renge
    int pos_min = obj.value("pos_min").toInt();
    int pos_max = obj.value("pos_max").toInt();
    setAbsoluteRange(pos_min, pos_max);

    return true;
}

void ACEILeverObject::saveToJSON(QJsonObject &obj) const
{
    obj["name"] = mName;
    obj["spring_return"] = hasSpringReturn();

    // Range
    obj["pos_min"] = mAbsoluteMin;
    obj["pos_max"] = mAbsoluteMax;
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

    for(LeverContactNode *c : mContactNodes)
    {
        c->setObjectName(mName);
    }
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

int ACEILeverObject::position() const
{
    return mPosition;
}

void ACEILeverObject::setPosition(int newPosition)
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

int ACEILeverObject::closestPosition(int angle, bool allowMiddle) const
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

void ACEILeverObject::timerEvent(QTimerEvent *e)
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

int ACEILeverObject::absoluteMax() const
{
    return mAbsoluteMax;
}

int ACEILeverObject::absoluteMin() const
{
    return mAbsoluteMin;
}

void ACEILeverObject::setAbsoluteRange(int newMin, int newMax)
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

    // Recalculate angle
    setAngle(mAngle);
}

void ACEILeverObject::addContactNode(LeverContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));

    mContactNodes.append(c);
    c->setObjectName(mName);
}

void ACEILeverObject::removeContactNode(LeverContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->lever() == this);

    mContactNodes.removeOne(c);
}

const LeverPositionDesc& ACEILeverObject::positionDesc() const
{
    return mPositionDesc;
}
