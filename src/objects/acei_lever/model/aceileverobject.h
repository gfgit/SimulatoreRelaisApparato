/**
 * src/objects/acei_lever/model/aceileverobject.h
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

#ifndef ACEILEVEROBJECT_H
#define ACEILEVEROBJECT_H

#include <QObject>

#include "../../../enums/aceileverposition.h"

class ACEILeverObject : public QObject
{
    Q_OBJECT
public:
    // This is a placeholder value to represent a range
    // Starting from previous position to next position
    static constexpr int MiddleAngle = 361;

    static constexpr int MaxSnapAngleDelta = 20;
    static constexpr int SpringTimerAngleDelta = 15;

    explicit ACEILeverObject(QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &newName);

    int angle() const;
    void setAngle(int newAngle);
    void setAngleTrySnap(int newAngle);

    ACEILeverPosition position() const;
    void setPosition(ACEILeverPosition newPosition);

    bool hasSpringReturn() const;
    void setHasSpringReturn(bool newHasSpringReturn);

    bool isPressed() const;
    void setPressed(bool newIsPressed);

    static constexpr int angleForPosition(ACEILeverPosition pos)
    {
        return PositionAngles[int(pos)];
    }

    static constexpr bool isPositionMiddle(ACEILeverPosition pos)
    {
        return angleForPosition(pos) == MiddleAngle;
    }

    static inline int middlePositionAngle(ACEILeverPosition pos)
    {
        Q_ASSERT(isPositionMiddle(pos));

        const int prevPosAngle = PositionAngles[int(pos) - 1];
        const int nextPosAngle = PositionAngles[int(pos) + 1];

        // Average
        return (prevPosAngle + nextPosAngle) / 2;
    }

    static inline ACEILeverPosition closestPosition(int angle, bool allowMiddle)
    {
        for(int i = 0; i < int(ACEILeverPosition::NPositions); i++)
        {
            if(PositionAngles[i] == MiddleAngle)
            {
                const int prevPosAngle = PositionAngles[i - 1];
                const int nextPosAngle = PositionAngles[i + 1];

                if(angle <= prevPosAngle || angle >= nextPosAngle)
                    continue;

                if(allowMiddle)
                    return ACEILeverPosition(i); // Return middle position

                if((angle - prevPosAngle) < (nextPosAngle - angle))
                    return ACEILeverPosition(i - 1); // Closest to prev

                return ACEILeverPosition(i + 1); // Closest to next
            }

            if(angle == PositionAngles[i])
                return ACEILeverPosition(i);
        }

        // Invalid
        return ACEILeverPosition::NPositions;
    }

    // Returns NPositions if cannot snap
    static inline ACEILeverPosition snapTarget(int angle)
    {
        // Skip Middle positions
        const ACEILeverPosition pos = closestPosition(angle, false);
        const int posAngle = angleForPosition(pos);

        if(qAbs(posAngle - angle) <= MaxSnapAngleDelta)
            return pos; // Snap to position

        // Do not snap
        return ACEILeverPosition::NPositions;
    }

    ACEILeverPosition absoluteMin() const;
    void setAbsoluteRange(ACEILeverPosition newMin, ACEILeverPosition newMax);

    ACEILeverPosition absoluteMax() const;

signals:
    void nameChanged(ACEILeverObject *self, const QString& newName);
    void angleChanged(ACEILeverObject *self, int newAngle);
    void positionChanged(ACEILeverObject *self, ACEILeverPosition newPosition);
    void pressedChanged(bool pressed);
    void changed(ACEILeverObject *self);

private:
    void timerEvent(QTimerEvent *e) override;

    void stopSpringTimer();
    void startSpringTimer();

private:
    // Angle in degrees clockwise from Normal (vertical up)
    static constexpr int PositionAngles[int(ACEILeverPosition::NPositions)]
    {
        -90, // Left
        MiddleAngle,
        0,   // Normal
        MiddleAngle,
        90   // Right
    };

    QString mName;

    int mAngle = angleForPosition(ACEILeverPosition::Normal);
    ACEILeverPosition mPosition = ACEILeverPosition::Normal;

    int springTimerId = 0;

    bool mHasSpringReturn = false;
    bool mIsPressed = false;

    ACEILeverPosition mAbsoluteMin = ACEILeverPosition(0);
    ACEILeverPosition mAbsoluteMax = ACEILeverPosition(int(ACEILeverPosition::NPositions) - 1);
};

#endif // ACEILEVEROBJECT_H
