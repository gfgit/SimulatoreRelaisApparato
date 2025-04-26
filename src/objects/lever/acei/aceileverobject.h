/**
 * src/objects/lever/acei/aceileverobject.h
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

#include "../../abstractsimulationobject.h"

enum class ACEILeverPosition
{
    Left = 0,
    Middle1,
    Vertical,
    Middle2,
    Right,
    NPositions
};

class LeverInterface;

class ACEILeverObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    explicit ACEILeverObject(AbstractSimulationObjectModel *m);
    ~ACEILeverObject();

    static constexpr QLatin1String Type = QLatin1String("acei_lever");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject& obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject& obj) const override;

    bool canSealLeftPosition() const;
    void setCanSealLeftPosition(bool newCanSealLeftPosition);

    bool isLeftPositionSealed() const;
    void setIsLeftPositionSealed(bool newIsLeftPositionSealed);

private:
    LeverInterface *leverInterface = nullptr;

    bool mCanSealLeftPosition = false;
    bool mIsLeftPositionSealed = false;
};

#endif // ACEILEVEROBJECT_H
