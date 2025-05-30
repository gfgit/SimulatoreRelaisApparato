/**
 * src/objects/button/genericbuttonobject.cpp
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

#include "genericbuttonobject.h"

#include "../interfaces/buttoninterface.h"
#include "../interfaces/mechanicalinterface.h"

#include <QTimerEvent>

#include <QCborMap>

GenericButtonObject::GenericButtonObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{
    buttonIface = new ButtonInterface(this);
    mechanicalIface = new MechanicalInterface(ButtonInterface::getStateDesc(),
                                              this);
    mechanicalIface->init();
    mechanicalIface->setUserCanChangeAbsoulteRange(false);
    mechanicalIface->setAllowedConditionTypes({MechanicalCondition::Type::RangePos,
                                               MechanicalCondition::Type::ExactPos});
    mechanicalIface->setLockablePositions({int(ButtonInterface::State::Pressed),
                                           int(ButtonInterface::State::Normal),
                                           int(ButtonInterface::State::Extracted)});

    mechanicalIface->addConditionSet(ButtonInterface::getStateDesc().name(int(ButtonInterface::State::Pressed)));
    mechanicalIface->setConditionSetRange(0,
                                          {int(ButtonInterface::State::Normal),
                                           int(ButtonInterface::State::Extracted)});
    mechanicalIface->addConditionSet(ButtonInterface::getStateDesc().name(int(ButtonInterface::State::Extracted)));
    mechanicalIface->setConditionSetRange(1,
                                          {int(ButtonInterface::State::Pressed),
                                           int(ButtonInterface::State::Normal)});
}

GenericButtonObject::~GenericButtonObject()
{
    mReturnTimeout.stop();

    delete buttonIface;
    buttonIface = nullptr;
}

QString GenericButtonObject::getType() const
{
    return Type;
}

bool GenericButtonObject::setReplicaState(const QCborMap &replicaState)
{
    buttonIface->setState(ButtonInterface::State(replicaState.value(QLatin1StringView("state")).toInteger()));
    return true;
}

void GenericButtonObject::getReplicaState(QCborMap &replicaState) const
{
    replicaState[QLatin1StringView("state")] = int(buttonIface->state());
}

void GenericButtonObject::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == mReturnTimeout.timerId())
    {
        // Reset button to Normal position
        mReturnTimeout.stop();
        buttonIface->setState(ButtonInterface::State::Normal);
        return;
    }

    AbstractSimulationObject::timerEvent(ev);
}

void GenericButtonObject::onInterfaceChanged(AbstractObjectInterface *iface, const QString &propName, const QVariant &value)
{
    if(iface == mechanicalIface)
    {
        if(propName == MechanicalInterface::LockRangePropName)
        {
            // Just set lock range
            setNewLockRange();
            return;
        }
        else if(propName == MechanicalInterface::PositionPropName)
        {
            // Mirror position
            const int pos = mechanicalIface->position();
            buttonIface->setState(ButtonInterface::State(pos));
        }
    }
    else if(iface == buttonIface)
    {
        if(propName == ButtonInterface::AbsoluteRangePropName)
        {
            // Sync ranges
            int minPos = int(ButtonInterface::State::Pressed);
            if(!buttonIface->canBePressed())
                minPos = int(ButtonInterface::State::Normal);
            int maxPos = int(ButtonInterface::State::Extracted);
            if(!buttonIface->canBeExtracted())
                maxPos = int(ButtonInterface::State::Normal);

            mechanicalIface->setAbsoluteRange(minPos, maxPos);
            setNewLockRange();
        }
        else if(propName == ButtonInterface::StatePropName)
        {
            if(buttonIface->mode() == ButtonInterface::Mode::ReturnNormalAfterTimeout
                    && buttonIface->state() != ButtonInterface::State::Normal)
            {
                startReturnTimer();
            }
            else
            {
                mReturnTimeout.stop();
            }

            mechanicalIface->setPosition(int(buttonIface->state()));
        }
        else if(propName == ButtonInterface::ModePropName)
        {
            if(buttonIface->mode() != ButtonInterface::Mode::ReturnNormalAfterTimeout)
            {
                if(mReturnTimeout.isActive())
                {
                    // Timer was active, reset button
                    buttonIface->setState(ButtonInterface::State::Normal);
                }

                mReturnTimeout.stop();
            }
        }
    }

    AbstractSimulationObject::onInterfaceChanged(iface, propName, value);
}

void GenericButtonObject::onReplicaModeChanged(bool on)
{
    if(!on)
    {
        switch(buttonIface->mode())
        {
        case ButtonInterface::Mode::ReturnNormalAfterTimeout:
        case ButtonInterface::Mode::ReturnNormalOnRelease:
        {
            // Reset button
            buttonIface->setState(ButtonInterface::State::Normal);
        }
        default:
            break;
        }
    }
}

void GenericButtonObject::startReturnTimer()
{
    mReturnTimeout.start(buttonIface->timeoutMillis(),
                         Qt::PreciseTimer, this);
}

void GenericButtonObject::setNewLockRange()
{
    // Set new locked range
    auto r = mechanicalIface->getCurrentLockRange();
    buttonIface->setAllowedLockPositions({ButtonInterface::State(r.first),
                                          ButtonInterface::State(r.second)});
    mechanicalIface->setLockedRange(r.first, r.second);

    // Check current position
    buttonIface->checkStateValidForLock();
}
