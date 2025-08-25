/**
 * src/objects/simple_activable/electromagnetobject.cpp
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

#include "electromagnetobject.h"

#include "../../circuits/nodes/electromagnetcontactnode.h"

ElectroMagnetObject::ElectroMagnetObject(AbstractSimulationObjectModel *m)
    : AbstractSimpleActivableObject{m}
{

}

ElectroMagnetObject::~ElectroMagnetObject()
{
    auto contactNodes = mContactNodes;
    for(ElectromagnetContactNode *c : contactNodes)
    {
        c->setMagnet(nullptr);
    }
}

QString ElectroMagnetObject::getType() const
{
    return Type;
}

int ElectroMagnetObject::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int count = AbstractSimulationObject::getReferencingNodes(result);
    count += mContactNodes.size();

    if(result)
    {
        for(ElectromagnetContactNode *node : std::as_const(mContactNodes))
            result->append(node);
    }

    return count;
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

void ElectroMagnetObject::onStateChangedInternal()
{
    for(ElectromagnetContactNode *node : std::as_const(mContactNodes))
        node->refreshContactState();
}

void ElectroMagnetObject::addContactNode(ElectromagnetContactNode *c)
{
    Q_ASSERT_X(!mContactNodes.contains(c),
               "addContactNode", "already added");

    mContactNodes.append(c);

    emit nodesChanged(this);
}

void ElectroMagnetObject::removeContactNode(ElectromagnetContactNode *c)
{
    Q_ASSERT_X(mContactNodes.contains(c),
               "removeContactNode", "not registered");
    Q_ASSERT_X(c->magnet() == this,
               "removeContactNode", "magnet does not match");

    mContactNodes.removeOne(c);

    emit nodesChanged(this);
}
