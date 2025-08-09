/**
 * src/circuits/nodes/buttoncontactnode.cpp
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

#include "buttoncontactnode.h"

#include "../../objects/abstractsimulationobjectmodel.h"
#include "../../objects/button/genericbuttonobject.h"
#include "../../objects/interfaces/buttoninterface.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

ButtonContactNode::ButtonContactNode(ModeManager *mgr, QObject *parent)
    : AbstractDeviatorNode{mgr, parent}
{
    // 3 sides
    // Common
    // Pressed (Deviator Up contact)
    // Normal  (Deviator Down contact)

    setSwapContactState(false);
    setAllowSwap(false);

    setHasCentralConnector(true);

    // Default state
    setContactState(false, false);
}

ButtonContactNode::~ButtonContactNode()
{
    setButton(nullptr);
}

bool ButtonContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractDeviatorNode::loadFromJSON(obj))
        return false;

    const QString buttonName = obj.value("button").toString();
    const QString buttonType = obj.value("button_type").toString();
    auto model = modeMgr()->modelForType(buttonType);

    if(model)
        setButton(model->getObjectByName(buttonName));
    else
        setButton(nullptr);

    const QString fmt("state_%1_contact_%2");
    for(qint64 i = 0; i < 3; i++)
    {
        setContactStateFor(i, 0, obj.value(fmt.arg(i).arg(0)).toBool());
        setContactStateFor(i, 1, obj.value(fmt.arg(i).arg(1)).toBool());
    }

    return true;
}

void ButtonContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractDeviatorNode::saveToJSON(obj);

    obj["button"] = mButton ? mButton->name() : QString();
    obj["button_type"] = mButton ? mButton->getType() : QString();

    const QString fmt("state_%1_contact_%2");
    for(int i = 0; i < 3; i++)
    {
        obj[fmt.arg(i).arg(0)] = getContactStateFor(i, 0);
        obj[fmt.arg(i).arg(1)] = getContactStateFor(i, 1);
    }
}

void ButtonContactNode::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty butProp;
    butProp.name = "button";
    butProp.prettyName = tr("Button");
    butProp.interface = ButtonInterface::IfaceType;
    result.append(butProp);
}

QString ButtonContactNode::nodeType() const
{
    return NodeType;
}

AbstractSimulationObject *ButtonContactNode::button() const
{
    return mButton;
}

void ButtonContactNode::setButton(AbstractSimulationObject *newButton)
{
    if(newButton && !newButton->getInterface<ButtonInterface>())
        return;

    if (mButton == newButton)
        return;

    if(mButton)
    {
        disconnect(mButton, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ButtonContactNode::onInterfacePropertyChanged);

        mButtonIface->removeContactNode(this);
        mButtonIface = nullptr;
    }

    mButton = newButton;

    if(mButton)
    {
        connect(mButton, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ButtonContactNode::onInterfacePropertyChanged);

        mButtonIface = mButton->getInterface<ButtonInterface>();
        mButtonIface->addContactNode(this);
    }

    emit buttonChanged(mButton);
    emit shapeChanged();
    refreshContactState();
    modeMgr()->setFileEdited();
}

ButtonInterface *ButtonContactNode::buttonIface() const
{
    return mButtonIface;
}

void ButtonContactNode::setContactStateFor(int buttonState, int contact, bool val)
{
    Q_ASSERT(buttonState >= 0 && buttonState < 3);
    Q_ASSERT(contact == 0 || contact == 1);

    if(contact == 0)
    {
        if(mFirstContactState[buttonState] == val)
            return;

        mFirstContactState[buttonState] = val;
    }
    else
    {
        if(mSecondContactState[buttonState] == val)
            return;

        mSecondContactState[buttonState] = val;
    }

    bool bothCanBeActive = false;
    for(int s = 0; s < 3; s++)
    {
        if(mFirstContactState[s] && mSecondContactState[s])
        {
            bothCanBeActive = true;
            break;
        }
    }
    setBothCanBeActive(bothCanBeActive);

    emit contactStateSettingsChanged();
    refreshContactState();
    modeMgr()->setFileEdited();
}

void ButtonContactNode::onInterfacePropertyChanged(const QString &ifaceName,
                                                   const QString &propName)
{
    if(ifaceName == ButtonInterface::IfaceType)
    {
        if(propName == ButtonInterface::StatePropName)
        {
            refreshContactState();
        }
    }
}

void ButtonContactNode::refreshContactState()
{
    ButtonInterface::State state = mButtonIface ?
                mButtonIface->state() :
                ButtonInterface::State::Normal;

    const bool straightContactOn = mFirstContactState[int(state)];
    bool centralContactOn = mSecondContactState[int(state)];

    // If we don't have second connector,
    // we fake it's state to be opposite of first connector
    // this is nicer to see in drawing and does not
    // affect circuit functioning
    if(!hasCentralConnector())
        centralContactOn = !straightContactOn;

    // Second contact is deviated one, first is straight
    setContactState(centralContactOn, straightContactOn);

    // Force button redraw
    emit deviatorStateChanged();
}
