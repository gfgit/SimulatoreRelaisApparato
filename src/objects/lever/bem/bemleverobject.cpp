/**
 * src/objects/lever/bem/bemleverobject.cpp
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

#include "bemleverobject.h"

#include "../../interfaces/leverinterface.h"
#include "../../interfaces/bemhandleinterface.h"

static const EnumDesc bem_mc_lever_posDesc =
{
    int(BEMLeverPositionMc::Normal),
    int(BEMLeverPositionMc::Middle3),
    int(BEMLeverPositionMc::Normal),
    "BEMLeverObject",
    {
        QT_TRANSLATE_NOOP("BEMLeverObject", "Normal"),
        {},
        QT_TRANSLATE_NOOP("BEMLeverObject", "Blocked"),
        {},
        QT_TRANSLATE_NOOP("BEMLeverObject", "Consensus")
    }
};

static const LeverAngleDesc bem_mc_lever_angleDesc =
{
    // Clockwise, even if lever is used counterclockwise
    {180, 180}, // Normal (Vertical down)
    {}, // Middle1
    {-60, -60}, // Blocked
    {}, // Middle2
    {+60, +60}, // Consensus
    {}  // Middle3
};

static const EnumDesc bem_mr_lever_posDesc =
{
    int(BEMLeverPositionMr::Normal),
    int(BEMLeverPositionMr::Middle5),
    int(BEMLeverPositionMr::Normal),
    "BEMLeverObject",
    {
        QT_TRANSLATE_NOOP("BEMLeverObject", "Normal"),
        {},
        QT_TRANSLATE_NOOP("BEMLeverObject", "Request Consensus"),
        {},
        QT_TRANSLATE_NOOP("BEMLeverObject", "Signals At Stop"),
        {},
        QT_TRANSLATE_NOOP("BEMLeverObject", "Activate 1° Cat Signal"),
        {},
        QT_TRANSLATE_NOOP("BEMLeverObject", "Activate Both Signals")
    }
};

static const LeverAngleDesc bem_mr_lever_angleDesc =
{
    {180, 180}, // VerticalDown
    {}, // Middle1
    {-60, -60}, // Request Consensus
    {}, // Middle2
    {-20, -20}, // Signals At Stop
    {}, // Middle3
    {+20, +20}, // Activate First Cat Signal
    {}, // Middle4
    {+60, +60}, // Activate Both Signals
    {}  // Middle5
};

BEMLeverObject::BEMLeverObject(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject(m)
{
    leverInterface = new LeverInterface(bem_mc_lever_posDesc,
                                        bem_mc_lever_angleDesc,
                                        this);
    leverInterface->setCanWarpAroundZero(true);
    leverInterface->init();

    leverInterface->setChangeRangeAllowed(false);

    bemInterface = new BEMHandleInterface(this);
}

BEMLeverObject::~BEMLeverObject()
{
    delete leverInterface;
    leverInterface = nullptr;

    delete bemInterface;
    bemInterface = nullptr;
}

QString BEMLeverObject::getType() const
{
    return Type;
}

void BEMLeverObject::onInterfaceChanged(AbstractObjectInterface *iface,
                                        const QString &propName,
                                        const QVariant &value)
{
    if(iface == bemInterface)
    {
        if(propName == BEMHandleInterface::LeverTypePropName)
        {
            leverInterface->setChangeRangeAllowed(true);

            if(bemInterface->leverType() == BEMHandleInterface::Consensus)
                leverInterface->setPositionDesc(bem_mc_lever_posDesc,
                                                bem_mc_lever_angleDesc);
            else
                leverInterface->setPositionDesc(bem_mr_lever_posDesc,
                                                bem_mr_lever_angleDesc);

            leverInterface->setChangeRangeAllowed(false);
        }
    }

    return AbstractSimulationObject::onInterfaceChanged(iface, propName, value);
}
