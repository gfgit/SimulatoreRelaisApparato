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

#include <QTimerEvent>

GenericButtonObject::GenericButtonObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{
    mButtonInterface = new ButtonInterface(this);
}

GenericButtonObject::~GenericButtonObject()
{
    stopReturnTimer();

    delete mButtonInterface;
    mButtonInterface = nullptr;
}

QString GenericButtonObject::getType() const
{
    return Type;
}

void GenericButtonObject::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == mReturnTimerId)
    {
        // Reset button to Normal position
        stopReturnTimer();
        mButtonInterface->setState(ButtonInterface::State::Normal);
        return;
    }

    AbstractSimulationObject::timerEvent(ev);
}

void GenericButtonObject::onInterfaceChanged(AbstractObjectInterface *iface, const QString &propName, const QVariant &value)
{
    if(iface == mButtonInterface)
    {
        if(propName == ButtonInterface::StatePropName)
        {
            if(mButtonInterface->mode() == ButtonInterface::Mode::ReturnNormalAfterTimeout
                    && mButtonInterface->state() != ButtonInterface::State::Normal)
            {
                startReturnTimer();
            }
            else
            {
                stopReturnTimer();
            }
        }
        else if(propName == ButtonInterface::ModePropName)
        {
            if(mButtonInterface->mode() != ButtonInterface::Mode::ReturnNormalAfterTimeout)
            {
                if(mReturnTimerId)
                {
                    // Timer was active, reset button
                    mButtonInterface->setState(ButtonInterface::State::Normal);
                }

                stopReturnTimer();
            }
        }
    }

    AbstractSimulationObject::onInterfaceChanged(iface, propName, value);
}

void GenericButtonObject::startReturnTimer()
{
    stopReturnTimer();

    // TODO: make configurable
    mReturnTimerId = startTimer(2000, Qt::CoarseTimer);
}

void GenericButtonObject::stopReturnTimer()
{
    if(!mReturnTimerId)
        return;

    killTimer(mReturnTimerId);
    mReturnTimerId = 0;
}
