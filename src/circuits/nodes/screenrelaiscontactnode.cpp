/**
 * src/circuits/nodes/screenrelaiscontactnode.cpp
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

#include "screenrelaiscontactnode.h"

#include "../../objects/screen_relais/model/screenrelais.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

ScreenRelaisContactNode::ScreenRelaisContactNode(ModeManager *mgr, QObject *parent)
    : AbstractDeviatorNode{mgr, parent}
{
    setHasCentralConnector(true);
    setCanChangeCentralConnector(false);
}

ScreenRelaisContactNode::~ScreenRelaisContactNode()
{
    setScreenRelais(nullptr);
}

bool ScreenRelaisContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractDeviatorNode::loadFromJSON(obj))
        return false;

    auto model = modeMgr()->modelForType(ScreenRelais::Type);
    if(model)
    {
        const QString relaisName = obj.value("screen_relais").toString();
        AbstractSimulationObject *relayObj = model->getObjectByName(relaisName);
        setScreenRelais(static_cast<ScreenRelais *>(relayObj));
    }
    else
        setScreenRelais(nullptr);

    setIsContactA(obj.value("contact_a").toBool(true));

    return true;
}

void ScreenRelaisContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);

    obj["screen_relais"] = mScreenRelais ? mScreenRelais->name() : QString();
    obj["contact_a"] = isContactA();
}

QString ScreenRelaisContactNode::nodeType() const
{
    return NodeType;
}

ScreenRelais *ScreenRelaisContactNode::screenRelais() const
{
    return mScreenRelais;
}

void ScreenRelaisContactNode::setScreenRelais(ScreenRelais *newRelais)
{
    if(mScreenRelais == newRelais)
        return;

    if(mScreenRelais)
    {
        mScreenRelais->removeContactNode(this);
    }

    mScreenRelais = newRelais;

    if(mScreenRelais)
    {
        mScreenRelais->addContactNode(this);
    }
    else
    {
        setState(ContactState::Middle);
    }

    emit relayChanged(mScreenRelais);

    modeMgr()->setFileEdited();
}

ScreenRelaisContactNode::ContactState ScreenRelaisContactNode::state() const
{
    return mState;
}

void ScreenRelaisContactNode::setState(ContactState newState)
{
    if (mState == newState)
        return;
    mState = newState;

    setContactState(mState == ContactState::Straight,
                    mState == ContactState::Reversed);
}

bool ScreenRelaisContactNode::isContactA() const
{
    return mIsContactA;
}

void ScreenRelaisContactNode::setIsContactA(bool newIsContactA)
{
    if(mIsContactA == newIsContactA)
        return;

    mIsContactA = newIsContactA;

    if(mScreenRelais)
    {
        const ScreenRelais::ContactState s = isContactA() ? mScreenRelais->getContactStateA() : mScreenRelais->getContactStateB();
        setState(ScreenRelaisContactNode::ContactState(s));
    }

    modeMgr()->setFileEdited();
}
