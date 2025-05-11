/**
 * src/objects/lever/ace_sasib/acesasiblever2positions.h
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

#ifndef ACE_SASIB_LEVER_5_POSITIONS_H
#define ACE_SASIB_LEVER_5_POSITIONS_H

#include "acesasiblevercommon.h"

class MechanicalInterface;
class ElectroMagnetObject;

enum class ACESasibLeverPosition2
{
    Reverse = 0,
    Middle1,
    CheckCircuitReverse,
    Middle2,
    WaitSwitchReverse,
    Middle3,
    WaitSwitchNormal,
    Middle4,
    CheckCircuitNormal,
    Middle5,
    Normal
};

class ACESasibLever2PosObject : public ACESasibLeverCommonObject
{
    Q_OBJECT
public:
    explicit ACESasibLever2PosObject(AbstractSimulationObjectModel *m);

    static constexpr QLatin1String Type = QLatin1String("ace_sasib_lever_2");
    QString getType() const override;

protected:
    void addElectromagnetLock() override;

    void updateButtonsMagnetLock() override;
};

#endif // ACE_SASIB_LEVER_5_POSITIONS_H
