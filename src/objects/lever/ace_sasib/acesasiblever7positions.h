/**
 * src/objects/lever/ace_sasib/acesasiblever7positions.h
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

#ifndef ACE_SASIB_LEVER_7_POSITIONS_H
#define ACE_SASIB_LEVER_7_POSITIONS_H

#include "acesasiblevercommon.h"

class MechanicalInterface;
class ElectroMagnetObject;

enum class ACESasibLeverPosition7
{
    TurnedForward = 0,
    Middle1,
    WaitLiberationForward,
    Middle2,
    WaitImmobilizationForward,
    Middle3,
    Normal,
    Middle4,
    WaitImmobilizationBackwards,
    Middle5,
    WaitLiberationBackwards,
    Middle6,
    TurnedBackwards
};

class ACESasibLever7PosObject : public ACESasibLeverCommonObject
{
    Q_OBJECT
public:
    explicit ACESasibLever7PosObject(AbstractSimulationObjectModel *m);

    static constexpr QLatin1String Type = QLatin1String("ace_sasib_lever_7");
    QString getType() const override;

protected:
    void addElectromagnetLock() override;
};

#endif // ACE_SASIB_LEVER_7_POSITIONS_H
