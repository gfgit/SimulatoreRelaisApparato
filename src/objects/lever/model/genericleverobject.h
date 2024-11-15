/**
 * src/objects/lever/model/genericleverobject.h
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

#ifndef GENERIC_LEVER_OBJECT_H
#define GENERIC_LEVER_OBJECT_H

#include "../../abstractsimulationobject.h"

#include "../../../enums/genericleverposition.h"

class LeverContactNode;

class QJsonObject;

class GenericLeverObject : public AbstractSimulationObject
{
    Q_OBJECT
private:
    static constexpr int MaxSnapAngleDelta = 20;
    static constexpr int SpringTimerAngleDelta = 15;

protected:
    GenericLeverObject(AbstractSimulationObjectModel *m, const LeverPositionDesc& desc);

public:
    ~GenericLeverObject();

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    QVector<AbstractCircuitNode*> nodes() const override;

    // State
    int angle() const;
    void setAngle(int newAngle);
    void setAngleTrySnap(int newAngle);

    int position() const;
    void setPosition(int newPosition);

    bool isPressed() const;
    void setPressed(bool newIsPressed);

    // Options
    bool hasSpringReturn() const;
    void setHasSpringReturn(bool newHasSpringReturn);

    int absoluteMin() const;
    int absoluteMax() const;
    void setAbsoluteRange(int newMin, int newMax);

    int normalPosition() const;
    void setNormalPosition(int newNormalPosition);


    const LeverPositionDesc &positionDesc() const;

    int closestPosition(int angle, bool allowMiddle) const;


    inline int angleForPosition(int pos) const
    {
        return mPositionDesc.angleFor(pos);
    }

    inline bool isPositionMiddle(int pos) const
    {
        return angleForPosition(pos) == LeverPositionDesc::MiddleAngle;
    }

    // Returns NPositions if cannot snap
    inline int snapTarget(int angle) const
    {
        // Skip Middle positions
        const int pos = closestPosition(angle, false);
        const int posAngle = angleForPosition(pos);

        if(qAbs(posAngle - angle) <= MaxSnapAngleDelta)
            return pos; // Snap to position

        // Do not snap
        return LeverPositionDesc::InvalidPosition;
    }

signals:
    void angleChanged(GenericLeverObject *self, int newAngle);
    void positionChanged(GenericLeverObject *self, int newPosition);
    void pressedChanged(bool pressed);

private:
    void timerEvent(QTimerEvent *e) override;

    void stopSpringTimer();
    void startSpringTimer();

    friend class LeverContactNode;
    void addContactNode(LeverContactNode *c);
    void removeContactNode(LeverContactNode *c);

private:
    QString mName;

    const LeverPositionDesc &mPositionDesc;

    int mAngle = 0;
    int mPosition = 0;
    int mNormalPosition = 0;

    int springTimerId = 0;

    bool mHasSpringReturn = false;
    bool mIsPressed = false;

    int mAbsoluteMin = 0;
    int mAbsoluteMax = 0;

    QVector<LeverContactNode *> mContactNodes;
};

#endif // GENERIC_LEVER_OBJECT_H
