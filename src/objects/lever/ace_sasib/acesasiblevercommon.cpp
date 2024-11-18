/**
 * src/objects/lever/ace_sasib/acesasiblevercommon.cpp
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

#include "acesasiblevercommon.h"

#include "../../interfaces/mechanicalinterface.h"

#include "../../simple_activable/electromagnet.h"

#include "../../abstractsimulationobjectmodel.h"

#include "../../../views/modemanager.h"

#include <QJsonObject>

ACESasibLeverCommonObject::ACESasibLeverCommonObject(AbstractSimulationObjectModel *m,
                                                     const LeverPositionDesc &desc)
    : GenericLeverObject(m, desc)
{
    mechanicalIface = new MechanicalInterface(this);
}

bool ACESasibLeverCommonObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!GenericLeverObject::loadFromJSON(obj, phase))
        return false;

    ElectroMagnetObject *magnet_ = nullptr;

    if(phase == LoadPhase::AllCreated)
    {
        auto model_ = model()->modeMgr()->modelForType(ElectroMagnetObject::Type);
        if(model_)
        {
            const QString magnetName = obj.value("electromagnet").toString();
            AbstractSimulationObject *magnetObj = model_->getObjectByName(magnetName);
            magnet_ = static_cast<ElectroMagnetObject *>(magnetObj);
        }
    }

    setMagnet(magnet_);

    return true;
}

void ACESasibLeverCommonObject::saveToJSON(QJsonObject &obj) const
{
    GenericLeverObject::saveToJSON(obj);

    obj["electromagnet"] = mMagnet ? mMagnet->name() : QString();
}

ElectroMagnetObject *ACESasibLeverCommonObject::magnet() const
{
    return mMagnet;
}

void ACESasibLeverCommonObject::setMagnet(ElectroMagnetObject *newMagnet)
{
    if(mMagnet == newMagnet)
        return;

    if(mMagnet)
    {
        disconnect(mMagnet, &AbstractSimpleActivableObject::stateChanged,
                   this, &ACESasibLeverCommonObject::updateElectroMagnetState);
        removeElectromagnetLock();
    }

    mMagnet = newMagnet;

    if(mMagnet)
    {
        connect(mMagnet, &AbstractSimpleActivableObject::stateChanged,
                this, &ACESasibLeverCommonObject::updateElectroMagnetState);

        if(mMagnet->state() == ElectroMagnetObject::State::Off)
            addElectromagnetLock();
    }

    emit settingsChanged(this);
}

void ACESasibLeverCommonObject::updateElectroMagnetState()
{
    if(!mMagnet)
        return;

    // Off magnet locks lever, On magnet frees lever
    if(mMagnet->state() == ElectroMagnetObject::State::Off)
        addElectromagnetLock();
    else
        removeElectromagnetLock();
}

void ACESasibLeverCommonObject::removeElectromagnetLock()
{
    mechanicalIface->setObjectLockConstraints(mMagnet, {});
}

AbstractObjectInterface *ACESasibLeverCommonObject::getInterface(const QString &ifaceName)
{
    if(ifaceName == MechanicalInterface::IfaceType)
        return mechanicalIface;

    return GenericLeverObject::getInterface(ifaceName);
}

void ACESasibLeverCommonObject::onInterfaceChanged(const QString &ifaceName)
{
    if(ifaceName == MechanicalInterface::IfaceType)
    {
        // Set new locked range
        auto r = mechanicalIface->getLockRangeForPos(position(),
                                                     absoluteMin(),
                                                     absoluteMax());
        setLockedRange(r.first, r.second);
        return;
    }
}

void ACESasibLeverCommonObject::recalculateLockedRange()
{
    // Off magnet locks lever, On magnet frees lever
    if(mMagnet && mMagnet->state() == ElectroMagnetObject::State::Off)
        addElectromagnetLock();
}
