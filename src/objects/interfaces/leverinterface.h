/**
 * src/objects/interfaces/leverinterface.h
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

#ifndef LEVERINTERFACE_H
#define LEVERINTERFACE_H

#include "abstractobjectinterface.h"

#include "../../utils/enum_desc.h"

class LeverContactNode;

struct LeverAngleDesc
{
    // Angle in degrees clockwise from vertical up
    // and must be in ascending order

    // This is a placeholder value to represent a range
    // Starting from previous position to next position
    static constexpr int MiddleAngle = 361;

    static constexpr int InvalidPosition = -1;

    struct Item
    {
        // Lever position angle
        int angle = MiddleAngle;

        // Lever contact preview angle
        int contactDrawingAngle = MiddleAngle;
    };

    QVector<Item> items;

    LeverAngleDesc(std::initializer_list<Item> args)
    {
        items = args;
    }
};

class LeverInterface : public AbstractObjectInterface
{
private:
    static constexpr int MaxSnapAngleDelta = 20;
    static constexpr int SpringTimerAngleDelta = 15;

public:
    // Property names
    static constexpr QLatin1String PressedPropName = QLatin1String("pressed");
    static constexpr QLatin1String PositionPropName = QLatin1String("position");
    static constexpr QLatin1String AnglePropName = QLatin1String("angle");
    static constexpr QLatin1String AbsoluteRangePropName = QLatin1String("abs_range");


    LeverInterface(const EnumDesc& posDesc,
                   const LeverAngleDesc& angleDesc,
                   AbstractSimulationObject *obj);
    ~LeverInterface();

    static constexpr QLatin1String IfaceType = QLatin1String("lever");
    QString ifaceType() override;

    QVector<AbstractCircuitNode*> nodes() const override;

    bool loadFromJSON(const QJsonObject &obj) override;
    void saveToJSON(QJsonObject &obj) const override;

    void init();

    // State
    int angle() const;
    void setAngle(int newAngle);
    void setAngleTrySnap(int newAngle);

    int position() const;
    void setPosition(int newPosition);

    inline const EnumDesc& positionDesc() const
    {
        return mPositionDesc;
    }

    bool isPressed() const;
    void setPressed(bool newIsPressed);

    int lockedMin() const;
    int lockedMax() const;

    // Options
    bool hasSpringReturn() const;
    void setHasSpringReturn(bool newHasSpringReturn);

    int absoluteMin() const;
    int absoluteMax() const;
    void setAbsoluteRange(int newMin, int newMax);

    int normalPosition() const;
    void setNormalPosition(int newNormalPosition);

    // Helpers
    int closestPosition(int angle, bool allowMiddle) const;

    inline int angleForPosition(int pos, bool preview = false) const
    {
        if(pos < mPositionDesc.minValue || pos > mPositionDesc.maxValue)
            return LeverAngleDesc::MiddleAngle;

        const auto& item = mAngleDesc.items.at(pos - mPositionDesc.minValue);
        return preview ? item.contactDrawingAngle : item.angle;
    }

    inline bool isPositionMiddle(int pos) const
    {
        return angleForPosition(pos) == LeverAngleDesc::MiddleAngle;
    }

    // Returns InvalidPosition if cannot snap
    inline int snapTarget(int angle) const
    {
        // Skip Middle positions
        const int pos = closestPosition(angle, false);
        const int posAngle = angleForPosition(pos);

        if(qAbs(posAngle - angle) <= MaxSnapAngleDelta)
            return pos; // Snap to position

        // Do not snap
        return LeverAngleDesc::InvalidPosition;
    }

    inline void setChangeRangeAllowed(bool val)
    {
        mCanSetRange = val;
    }

    inline bool canChangeRange() const
    {
        return mCanSetRange;
    }

    inline void setChangeSpringAllowed(bool val)
    {
        mCanChangeSpring = val;
    }

    inline bool canChangeSpring() const
    {
        return mCanChangeSpring;
    }

    void setLockedRange(int newMin, int newMax);
    void checkPositionValidForLock();

protected:
    bool timerEvent(const int timerId) override;

    inline bool isPositionValidForLock(int pos) const
    {
        return pos >= mLockedMin && pos <= mLockedMax;
    }

private:
    void stopSpringTimer();
    void startSpringTimer();

    friend class LeverContactNode;
    void addContactNode(LeverContactNode *c);
    void removeContactNode(LeverContactNode *c);

private:
    const EnumDesc mPositionDesc;
    const LeverAngleDesc mAngleDesc;

    int mAngle = 0;
    int mPosition = 0;
    int mNormalPosition = 0;

    int springTimerId = 0;

    bool mHasSpringReturn = false;
    bool mIsPressed = false;

    int mAbsoluteMin = 0;
    int mAbsoluteMax = 0;
    int mLockedMin = 0;
    int mLockedMax = 0;

    bool mCanSetRange = true;
    bool mCanChangeSpring = false;

    QVector<LeverContactNode *> mContactNodes;
};

#endif // LEVERINTERFACE_H
