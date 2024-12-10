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

#include "../../relais/model/abstractrelais.h"

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

    recalculateLockedRange();
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

            fixBothInMiddlePosition();

            recalculateLockedRange();
        }
        else if(propName == BEMHandleInterface::TwinLeverPropName)
        {
            fixBothInMiddlePosition();

            recalculateLockedRange();
        }
        else if(propName == BEMHandleInterface::LibRelayPropName)
        {
            setLiberationRelay(bemInterface->liberationRelay());
        }
    }
    else if(iface == leverInterface)
    {
        if(propName == LeverInterface::PositionPropName)
        {
            recalculateLockedRange();

            if(bemInterface->getTwinHandle())
            {
                // Recalculate also other lever range
                // because twin levers are interconnected
                BEMLeverObject *otherObj = qobject_cast<BEMLeverObject *>(bemInterface->getTwinHandle()->object());
                if(otherObj)
                    otherObj->recalculateLockedRange();
            }
        }
    }

    return AbstractSimulationObject::onInterfaceChanged(iface, propName, value);
}

void BEMLeverObject::onLiberationRelayStateChanged()
{
    if(bemInterface->leverType() == BEMHandleInterface::LeverType::Consensus)
        recalculateLockedRange();
}

void BEMLeverObject::recalculateLockedRange()
{
    if(bemInterface->getTwinHandle())
    {
        LeverInterface *otherLeverIface = bemInterface->getTwinHandle()->object()->getInterface<LeverInterface>();
        if(otherLeverIface->isPositionMiddle(otherLeverIface->position()))
        {
            // Other lever is in middle position
            // We cannot be in middle position at same time

            const int currentPos = leverInterface->position();
            Q_ASSERT(!leverInterface->isPositionMiddle(currentPos));

            // Lock lever in current position until other lever
            // is not in middle position anymore
            leverInterface->setLockedRange(currentPos, currentPos);
            return;
        }
    }

    if(bemInterface->leverType() == BEMHandleInterface::Consensus)
    {
        // This handle can only rotate counter-clockwise
        BEMLeverPositionMc pos = BEMLeverPositionMc(leverInterface->position());

        BEMLeverPositionMc lockedMin;
        BEMLeverPositionMc lockedMax;
        switch (pos)
        {
        case BEMLeverPositionMc::Normal:
        case BEMLeverPositionMc::Middle3:
        {
            lockedMin = BEMLeverPositionMc::Consensus;
            lockedMax = BEMLeverPositionMc::Normal;
            break;
        }
        case BEMLeverPositionMc::Consensus:
        case BEMLeverPositionMc::Middle2:
        {
            lockedMin = BEMLeverPositionMc::Blocked;
            lockedMax = BEMLeverPositionMc::Consensus;
            break;
        }
        case BEMLeverPositionMc::Blocked:
        {
            // TODO: artificial liberation
            // If liberation relay is not fully Up, lever stays locked
            bool blocked = mLiberationRelay && mLiberationRelay->state() != AbstractRelais::State::Up;

            lockedMin = blocked ? BEMLeverPositionMc::Blocked : BEMLeverPositionMc::Normal;
            lockedMax = BEMLeverPositionMc::Blocked;
            break;
        }
        case BEMLeverPositionMc::Middle1:
        {
            // We already passed Blocked position
            // so we can go up to Normal
            lockedMin = BEMLeverPositionMc::Normal;
            lockedMax = BEMLeverPositionMc::Blocked;
            break;
        }
        default:
            Q_ASSERT(false);
            break;
        }

        leverInterface->setLockedRange(int(lockedMin), int(lockedMax));
    }
    else
    {
        // This handle can only rotate clockwise
        BEMLeverPositionMr pos = BEMLeverPositionMr(leverInterface->position());

        BEMLeverPositionMr lockedMin;
        BEMLeverPositionMr lockedMax;
        switch (pos)
        {
        case BEMLeverPositionMr::Normal:
        case BEMLeverPositionMr::Middle1:
            lockedMin = BEMLeverPositionMr::Normal;
            lockedMax = BEMLeverPositionMr::RequestConsensus;
            break;
        case BEMLeverPositionMr::RequestConsensus:
        case BEMLeverPositionMr::Middle2:
            lockedMin = BEMLeverPositionMr::RequestConsensus;
            lockedMax = BEMLeverPositionMr::SignalsAtStop;
            break;
        case BEMLeverPositionMr::SignalsAtStop:
        case BEMLeverPositionMr::Middle3:
            lockedMin = BEMLeverPositionMr::SignalsAtStop;
            lockedMax = BEMLeverPositionMr::ActivateFirstCatSignal;
            break;
        case BEMLeverPositionMr::ActivateFirstCatSignal:
        case BEMLeverPositionMr::Middle4:
            // Allow going back
            lockedMin = BEMLeverPositionMr::SignalsAtStop;
            lockedMax = BEMLeverPositionMr::ActivateBothSignals;
            break;
        case BEMLeverPositionMr::ActivateBothSignals:
        case BEMLeverPositionMr::Middle5:
            // Allow going back
            lockedMin = BEMLeverPositionMr::ActivateFirstCatSignal;
            lockedMax = BEMLeverPositionMr::Normal;
            break;
        default:
            Q_ASSERT(false);
            break;
        }

        leverInterface->setLockedRange(int(lockedMin), int(lockedMax));
    }
}

void BEMLeverObject::fixBothInMiddlePosition()
{
    if(bemInterface->leverType() != BEMHandleInterface::LeverType::Consensus)
        return;

    if(bemInterface->getTwinHandle())
        return;

    Q_ASSERT(bemInterface->getTwinHandle()->leverType() == BEMHandleInterface::LeverType::Request);

    // We are Consensus, twin is Request
    // Cannot be both in middle position, we ajdust request in case this happen
    if(!leverInterface->isPositionMiddle(leverInterface->position()))
        return;

    // Adjust other lever
    LeverInterface *otherLeverIface = bemInterface->getTwinHandle()->object()->getInterface<LeverInterface>();
    if(!otherLeverIface)
        return; // Error

    BEMLeverPositionMr otherPos = BEMLeverPositionMr(otherLeverIface->position());
    if(!otherLeverIface->isPositionMiddle(int(otherPos)))
        return; // Already not in middle position

    // Both lever are in middle position, adjust Request lever
    // Go to next position which is not middle
    if(otherPos == BEMLeverPositionMr::Middle5)
        otherPos = BEMLeverPositionMr::Normal; // Warp, go to first
    else
        otherPos = BEMLeverPositionMr(int(otherPos) + 1); // Next

    otherLeverIface->setAngle(otherLeverIface->angleForPosition(int(otherPos)));
    otherLeverIface->setPosition(int(otherPos));
}

AbstractRelais *BEMLeverObject::liberationRelay() const
{
    return mLiberationRelay;
}

void BEMLeverObject::setLiberationRelay(AbstractRelais *newLiberationRelay)
{
    if(mLiberationRelay == newLiberationRelay)
        return;

    if(mLiberationRelay)
    {
        disconnect(mLiberationRelay, &AbstractRelais::stateChanged,
                   this, &BEMLeverObject::onLiberationRelayStateChanged);
    }

    mLiberationRelay = newLiberationRelay;

    if(mLiberationRelay)
    {
        connect(mLiberationRelay, &AbstractRelais::stateChanged,
                this, &BEMLeverObject::onLiberationRelayStateChanged);
    }

    onLiberationRelayStateChanged();
}
