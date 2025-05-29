/**
 * src/objects/simple_activable/lightbulbobject.cpp
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

#include "lightbulbobject.h"

#include <QCborMap>

LightBulbObject::LightBulbObject(AbstractSimulationObjectModel *m)
    : AbstractSimpleActivableObject{m}
{

}

QString LightBulbObject::getType() const
{
    return Type;
}

bool LightBulbObject::setReplicaState(const QCborMap &replicaState)
{
    State newState = State(replicaState.value(QLatin1StringView("state")).toInteger());
    if(newState != mReplicaState)
    {
        mReplicaState = newState;
        emit stateChanged(this);
    }

    return true;
}

void LightBulbObject::getReplicaState(QCborMap &replicaState) const
{
    replicaState[QLatin1StringView("state")] = int(state());
}

void LightBulbObject::onReplicaModeChanged(bool on)
{
    if(!on)
        mReplicaState = State::Off;
    emit stateChanged(this);
}

AbstractSimpleActivableObject::State LightBulbObject::state() const
{
    if(isRemoteReplica())
        return mReplicaState;
    return AbstractSimpleActivableObject::state();
}
