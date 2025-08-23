/**
 * src/circuits/nodes/electromagnetcontactnode.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#include "electromagnetcontactnode.h"

#include "../../objects/abstractsimulationobjectmodel.h"
#include "../../objects/simple_activable/electromagnet.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

ElectromagnetContactNode::ElectromagnetContactNode(ModeManager *mgr, QObject *parent)
    : AbstractDeviatorNode{mgr, parent}
{
    // 3 sides
    // Common
    // Pressed (Deviator Up contact)
    // Normal  (Deviator Down contact)

    setSwapContactState(false);
    setAllowSwap(true);

    setHasCentralConnector(false);

    // Default state
    setContactState(false, false);
}

ElectromagnetContactNode::~ElectromagnetContactNode()
{
    setMagnet(nullptr);
}

bool ElectromagnetContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractDeviatorNode::loadFromJSON(obj))
        return false;

    const QString buttonName = obj.value("magnet").toString();
    auto model = modeMgr()->modelForType(ElectroMagnetObject::Type);

    if(model)
        setMagnet(static_cast<ElectroMagnetObject *>(model->getObjectByName(buttonName)));
    else
        setMagnet(nullptr);

    return true;
}

void ElectromagnetContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);

    obj["magnet"] = mMagnet ? mMagnet->name() : QString();
}

void ElectromagnetContactNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty magnetProp;
    magnetProp.name = "magnet";
    magnetProp.prettyName = tr("Magnet");
    magnetProp.types = {ElectroMagnetObject::Type};
    result.append(magnetProp);
}

QString ElectromagnetContactNode::nodeType() const
{
    return NodeType;
}

ElectroMagnetObject *ElectromagnetContactNode::magnet() const
{
    return mMagnet;
}

void ElectromagnetContactNode::setMagnet(ElectroMagnetObject *newMagnet)
{
    if (mMagnet == newMagnet)
        return;

    if(mMagnet)
    {
        mMagnet->removeContactNode(this);
    }

    mMagnet = newMagnet;

    if(mMagnet)
    {
        mMagnet->addContactNode(this);
    }

    emit magnetChanged(mMagnet);
    emit shapeChanged();
    refreshContactState();
    modeMgr()->setFileEdited();
}

void ElectromagnetContactNode::refreshContactState()
{
    const bool straightContactOn = mMagnet && mMagnet->state() == ElectroMagnetObject::State::On;
    const bool centralContactOn = mMagnet && mMagnet->state() == ElectroMagnetObject::State::Off;

    // Second contact is deviated one, first is straight
    setContactState(centralContactOn, straightContactOn);
}

