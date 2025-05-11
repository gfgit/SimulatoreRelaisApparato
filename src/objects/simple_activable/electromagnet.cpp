/**
 * src/objects/simple_activable/electromagnet.cpp
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

#include "electromagnet.h"

ElectroMagnetObject::ElectroMagnetObject(AbstractSimulationObjectModel *m)
    : AbstractSimpleActivableObject{m}
{

}

QString ElectroMagnetObject::getType() const
{
    return Type;
}

AbstractSimpleActivableObject::State ElectroMagnetObject::electricalState() const
{
    return AbstractSimpleActivableObject::state();
}

AbstractSimpleActivableObject::State ElectroMagnetObject::state() const
{
    if(mForcedOff)
        return State::Off;
    if(mForcedOn)
        return State::On;
    return electricalState();
}

void ElectroMagnetObject::setForcedOff(bool newForcedOff)
{
    const State oldState = state();
    mForcedOff = newForcedOff;

    if(oldState != state())
    {
        onStateChangedInternal();
        emit stateChanged(this);
    }
}

void ElectroMagnetObject::setForcedOn(bool newForcedOn)
{
    const State oldState = state();
    mForcedOn = newForcedOn;

    if(oldState != state())
    {
        onStateChangedInternal();
        emit stateChanged(this);
    }
}
