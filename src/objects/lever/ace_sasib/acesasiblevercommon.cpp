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
#include "../../interfaces/leverinterface.h"
#include "../../interfaces/sasibaceleverextrainterface.h"

#include "../../simple_activable/electromagnet.h"

#include "../../abstractsimulationobjectmodel.h"

#include "../../../views/modemanager.h"

#include <QJsonObject>

ACESasibLeverCommonObject::ACESasibLeverCommonObject(AbstractSimulationObjectModel *m,
                                                     const EnumDesc &positionDesc,
                                                     const LeverAngleDesc &angleDesc)
    : AbstractSimulationObject(m)
{
    mechanicalIface = new MechanicalInterface(positionDesc,
                                              this);
    leverInterface = new LeverInterface(positionDesc,
                                        angleDesc,
                                        this);

    sasibInterface = new SasibACELeverExtraInterface(this);

    // After all interfaces are constructed
    mechanicalIface->init();
    leverInterface->init();
}

ACESasibLeverCommonObject::~ACESasibLeverCommonObject()
{
    delete mechanicalIface;
    mechanicalIface = nullptr;

    delete leverInterface;
    leverInterface = nullptr;
}

bool ACESasibLeverCommonObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
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
    AbstractSimulationObject::saveToJSON(obj);

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

void ACESasibLeverCommonObject::onInterfaceChanged(AbstractObjectInterface *iface, const QString &propName, const QVariant &value)
{
    if(iface == mechanicalIface)
    {
        if(propName == MechanicalInterface::AbsoluteRangePropName)
        {
            // Sync ranges
            leverInterface->setAbsoluteRange(mechanicalIface->absoluteMin(),
                                             mechanicalIface->absoluteMax());
            setNewLockRange();
            return;
        }
        else if(propName == MechanicalInterface::LockRangePropName)
        {
            // Just set lock range
            setNewLockRange();
            return;
        }
        else if(propName == MechanicalInterface::PositionPropName)
        {
            // Mirror positions, let lever update locked range
            const int pos = mechanicalIface->position();
            int newLeverAngle = leverInterface->angleForPosition(pos);
            if(leverInterface->isPositionMiddle(pos))
            {
                // Set an angle halfway between 2 positions
                const int prevAngle = leverInterface->angleForPosition(pos - 1);
                const int nextAngle = leverInterface->angleForPosition(pos + 1);

                newLeverAngle = (prevAngle + nextAngle) / 2;
            }

            leverInterface->setAngle(newLeverAngle);

            // Check if position change was rejected
            if(mechanicalIface->position() != leverInterface->position())
                mechanicalIface->setPosition(leverInterface->position());
        }
    }
    else if(iface == leverInterface)
    {
        if(propName == LeverInterface::AbsoluteRangePropName)
        {
            // Sync ranges
            mechanicalIface->setAbsoluteRange(leverInterface->absoluteMin(),
                                              leverInterface->absoluteMax());
            setNewLockRange();
        }
        else if(propName == LeverInterface::PositionPropName)
        {
            // Mirror positions
            mechanicalIface->setPosition(leverInterface->position());

            // Recalculate lock range
            recalculateLockedRange();
        }
    }

    AbstractSimulationObject::onInterfaceChanged(iface, propName, value);
}

void ACESasibLeverCommonObject::removeElectromagnetLock()
{
    mechanicalIface->setObjectLockConstraints(mMagnet, {});
}

void ACESasibLeverCommonObject::recalculateLockedRange()
{
    // Off magnet locks lever, On magnet frees lever
    if(mMagnet && mMagnet->state() == ElectroMagnetObject::State::Off)
        addElectromagnetLock();
}

void ACESasibLeverCommonObject::setNewLockRange()
{
    // Set new locked range
    auto r = mechanicalIface->getLockRangeForPos(leverInterface->position(),
                                                 leverInterface->absoluteMin(),
                                                 leverInterface->absoluteMax());
    leverInterface->setLockedRange(r.first, r.second);
    mechanicalIface->setLockedRange(r.first, r.second);

    // Check current position
    leverInterface->checkPositionValidForLock();
}
