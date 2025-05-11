/**
 * src/objects/interfaces/sasibaceleverextrainterface.cpp
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

#include "sasibaceleverextrainterface.h"

#include "../abstractsimulationobject.h"
#include "../abstractsimulationobjectmodel.h"

#include "../lever/ace_sasib/acesasiblevercommon.h"

#include "../../views/modemanager.h"

#include "buttoninterface.h"
#include "mechanicalinterface.h"

#include <QJsonObject>

SasibACELeverExtraInterface::SasibACELeverExtraInterface(AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
{

}

SasibACELeverExtraInterface::~SasibACELeverExtraInterface()
{
    setButton(nullptr, Button::Left);
    setButton(nullptr, Button::Right);
}

QString SasibACELeverExtraInterface::ifaceType()
{
    return IfaceType;
}

bool SasibACELeverExtraInterface::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractObjectInterface::loadFromJSON(obj, phase))
        return false;

    if(phase == LoadPhase::Creation)
    {
        setRightButtonSwitchElectroMagnet(obj.value("right_but_magnet").toBool(false));
    }
    else if(phase == LoadPhase::AllCreated)
    {
        // Left Button
        AbstractSimulationObject *butObj = nullptr;
        const QString leftButName = obj.value(LeftButPropName).toString();
        const QString leftButType = obj.value(LeftButPropName + "_type").toString();

        AbstractSimulationObjectModel *butModel = object()->model()->modeMgr()->modelForType(leftButType);
        if(butModel)
            butObj = butModel->getObjectByName(leftButName);

        setButton(butObj, Button::Left);

        butObj = nullptr;
        const QString rightButName = obj.value(RightButPropName).toString();
        const QString rightButType = obj.value(RightButPropName + "_type").toString();

        butModel = object()->model()->modeMgr()->modelForType(rightButType);
        if(butModel)
            butObj = butModel->getObjectByName(rightButName);

        setButton(butObj, Button::Right);
    }

    return true;
}

void SasibACELeverExtraInterface::saveToJSON(QJsonObject &obj) const
{
    AbstractObjectInterface::saveToJSON(obj);

    obj[LeftButPropName] = mLeftButton ? mLeftButton->name() : QString();
    obj[LeftButPropName + "_type"] = mLeftButton ? mLeftButton->getType() : QString();

    obj[RightButPropName] = mRightButton ? mRightButton->name() : QString();
    obj[RightButPropName + "_type"] = mRightButton ? mRightButton->getType() : QString();

    obj["right_but_magnet"] = rightButtonSwitchElectroMagnet();
}

AbstractSimulationObject *SasibACELeverExtraInterface::getButton(Button whichBut) const
{
    AbstractSimulationObject *but = (whichBut == Button::Left) ? mLeftButton : mRightButton;
    return but;
}

void SasibACELeverExtraInterface::setButton(AbstractSimulationObject *newButton, Button whichBut)
{
    if(newButton)
    {
        if(mLeftButton == newButton || mRightButton == newButton)
            return;

        if(!newButton->getInterface<ButtonInterface>())
            return;
    }

    AbstractSimulationObject *&but = (whichBut == Button::Left) ? mLeftButton : mRightButton;
    if(newButton == but)
        return;

    if(but)
    {
        untrackObject(but);

        setButtonLocked(false, whichBut);
    }

    but = newButton;

    if(but)
    {
        trackObject(but);

        ButtonInterface *butIface = but->getInterface<ButtonInterface>();
        butIface->setCanBePressed(true);
    }

    if(whichBut == Button::Right)
        updateMagnetState();

    emitChanged((whichBut == Button::Left) ? LeftButPropName : RightButPropName,
                QVariant());
    emit object()->settingsChanged(object());
}

void SasibACELeverExtraInterface::setButtonLocked(bool lock, Button whichBut)
{
    constexpr MechanicalInterface::LockRange ButLock = std::make_pair(int(ButtonInterface::State::Normal),
                                                                      int(ButtonInterface::State::Extracted));

    AbstractSimulationObject *but = getButton(whichBut);
    if(!but)
        return;

    MechanicalInterface *mechIface = but->getInterface<MechanicalInterface>();
    if(!mechIface)
        return;

    if(lock)
        mechIface->setObjectLockConstraints(object(), {ButLock});
    else
        mechIface->setObjectLockConstraints(object(), {});
}

void SasibACELeverExtraInterface::onTrackedObjectDestroyed(AbstractSimulationObject *obj)
{
    if(obj == mRightButton)
        setButton(nullptr, Button::Right);
    else if(obj == mLeftButton)
        setButton(nullptr, Button::Left);
}

void SasibACELeverExtraInterface::updateMagnetState()
{
    ButtonInterface *butIface = nullptr;
    if(mRightButtonSwitchElectroMagnet && mRightButton)
        butIface = mRightButton->getInterface<ButtonInterface>();

    auto lever = static_cast<ACESasibLeverCommonObject *>(object());
    if(butIface && butIface->state() == ButtonInterface::State::Pressed)
        lever->forceMagnetUp(true);
    else
        lever->forceMagnetUp(false);
}

bool SasibACELeverExtraInterface::rightButtonSwitchElectroMagnet() const
{
    return mRightButtonSwitchElectroMagnet;
}

void SasibACELeverExtraInterface::setRightButtonSwitchElectroMagnet(bool val)
{
    if(val == mRightButtonSwitchElectroMagnet)
        return;

    mRightButtonSwitchElectroMagnet = val;
    emit object()->settingsChanged(object());

    updateMagnetState();
}
