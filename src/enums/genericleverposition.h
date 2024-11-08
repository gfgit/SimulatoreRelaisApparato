/**
 * src/enums/genericleverposition.h
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

#ifndef GENERICLEVERPOSITION_H
#define GENERICLEVERPOSITION_H

#include <memory>
#include <QVector>

enum class LeverPositionConditionType
{
    Exact = 0,
    FromTo = 1,
    NTypes
};

struct LeverPositionCondition
{
    int positionFrom = 0; // Or exact position
    int positionTo = 0;
    LeverPositionConditionType type = LeverPositionConditionType::Exact;
};

typedef QVector<LeverPositionCondition> LeverPositionConditionSet;

struct LeverPositionDesc
{
public:
    // Angle in degrees clockwise from Normal (vertical up)

    // This is a placeholder value to represent a range
    // Starting from previous position to next position
    static constexpr int MiddleAngle = 361;

    static constexpr int InvalidPosition = -1;

    typedef QString (*TranslateFunc)(const char *name);

    struct Item
    {
        // Lever position angle
        int angle = MiddleAngle;

        // Lever contact preview angle
        int contactDrawingAngle = MiddleAngle;

        // Position name
        const char *nameId = nullptr;

        inline bool isMiddle() const
        {
            return angle == MiddleAngle;
        }
    };

private:
    TranslateFunc mTranslateFunc = nullptr;
    const Item *mItems = nullptr;
    const int mSize = 0;

public:
    template<int N>
    LeverPositionDesc(const Item (&arr)[N],
                      int defaultPos_,
                      TranslateFunc f)
        : mItems(arr)
        , mSize(N)
        , defaultPositionIdx(defaultPos_)
        , mTranslateFunc(f)
    {
    }

    const int defaultPositionIdx = 0;

    inline QString nameFor(int idx) const
    {
        if(idx < 0 || idx >= mSize)
            return QString();

        if(!mItems[idx].nameId)
            return QString();

        if(mTranslateFunc)
            return mTranslateFunc(mItems[idx].nameId);

        return QLatin1String(mItems[idx].nameId);
    }

    inline int angleFor(int idx) const
    {
        if(idx < 0 || idx >= mSize)
            return MiddleAngle;
        return mItems[idx].angle;
    }

    inline int previewAngleFor(int idx) const
    {
        if(idx < 0 || idx >= mSize)
            return MiddleAngle;
        return mItems[idx].contactDrawingAngle;
    }

    inline int isMiddle(int idx) const
    {
        if(idx < 0 || idx >= mSize)
            return true;
        return mItems[idx].isMiddle();
    }

    inline int size() const
    {
        return mSize;
    }

    inline int maxPosition() const
    {
        return mSize - 1;
    }

    inline int exactPositionsCount() const
    {
        // Return number of positions without middle
        return (((mSize - 1) / 2) + 1);
    }
};

#endif // GENERICLEVERPOSITION_H
