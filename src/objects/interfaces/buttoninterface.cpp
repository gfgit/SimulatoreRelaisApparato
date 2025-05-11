/**
 * src/objects/interfaces/buttoninterface.cpp
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

#include "buttoninterface.h"
#include "../abstractsimulationobject.h"

#include "../../circuits/nodes/buttoncontactnode.h"

#include "../../utils/enum_desc.h"

#include <QJsonObject>

static const EnumDesc button_state_desc =
{
    int(ButtonInterface::State::Pressed),
    int(ButtonInterface::State::Extracted),
    int(ButtonInterface::State::Normal),
    "GenericButtonObject",
    {
        QT_TRANSLATE_NOOP("GenericButtonObject", "Pressed"),
        QT_TRANSLATE_NOOP("GenericButtonObject", "Normal"),
        QT_TRANSLATE_NOOP("GenericButtonObject", "Extracted")
    }
};

static const EnumDesc button_mode_desc =
{
    int(ButtonInterface::Mode::ReturnNormalOnRelease),
    int(ButtonInterface::Mode::ReturnNormalAfterTimeout),
    int(ButtonInterface::Mode::ReturnNormalOnRelease),
    "GenericButtonObject",
    {
        QT_TRANSLATE_NOOP("GenericButtonObject", "Return Normal on release"),
        QT_TRANSLATE_NOOP("GenericButtonObject", "Stay in last position"),
        QT_TRANSLATE_NOOP("GenericButtonObject", "Return Normal after timeout")
    }
};

ButtonInterface::ButtonInterface(AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
{

}

ButtonInterface::~ButtonInterface()
{
    auto contactNodes = mContactNodes;
    for(ButtonContactNode *c : contactNodes)
    {
        c->setButton(nullptr);
    }
}

QString ButtonInterface::ifaceType()
{
    return IfaceType;
}

int ButtonInterface::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    if(result)
    {
        for(auto item : mContactNodes)
            result->append(item);
    }

    return mContactNodes.size();
}

bool ButtonInterface::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractObjectInterface::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    // Default to allow chosing one
    setCanBeExtracted(true);
    setCanBePressed(true);

    // Actual value
    setCanBePressed(obj.value("can_press").toBool(true));
    setCanBeExtracted(obj.value("can_extract").toBool(false));

    setMode(Mode(obj.value("mode").toInt(int(Mode::ReturnNormalOnRelease))));

    return true;
}

void ButtonInterface::saveToJSON(QJsonObject &obj) const
{
    AbstractObjectInterface::saveToJSON(obj);

    obj["can_press"] = mCanBePressed;
    obj["can_extract"] = mCanBeExtracted;
    obj["mode"] = int(mMode);
}

ButtonInterface::State ButtonInterface::state() const
{
    return mState;
}

void ButtonInterface::setState(State newState)
{
    if(newState == mState)
        return;

    const auto range = allowedLockPositions();
    if(newState < range.first || newState > range.second)
        return;

    mState = newState;

    emitChanged(StatePropName, int(mState));
    emit mObject->stateChanged(mObject);
}

void ButtonInterface::addContactNode(ButtonContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));

    mContactNodes.append(c);

    emit mObject->nodesChanged(mObject);
}

void ButtonInterface::removeContactNode(ButtonContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->button() == mObject);

    mContactNodes.removeOne(c);

    emit mObject->nodesChanged(mObject);
}

void ButtonInterface::checkStateValidForLock()
{
    const auto range = allowedLockPositions();
    if(state() < range.first)
        setState(range.first);
    else if(state() > range.second)
        setState(range.second);
}

ButtonInterface::Mode ButtonInterface::mode() const
{
    return mMode;
}

void ButtonInterface::setMode(Mode newMode)
{
    if(mMode == newMode)
        return;

    mMode = newMode;
    emitChanged(ModePropName, QVariant());
    emit mObject->settingsChanged(mObject);
}

const EnumDesc &ButtonInterface::getStateDesc()
{
    return button_state_desc;
}

const EnumDesc &ButtonInterface::getModeDesc()
{
    return button_mode_desc;
}

bool ButtonInterface::canBePressed() const
{
    return mCanBePressed;
}

void ButtonInterface::setCanBePressed(bool newCanBePressed)
{
    if(mCanBePressed == newCanBePressed)
        return;

    if(!mCanBeExtracted && !newCanBePressed)
        return; // At least one must be on

    mCanBePressed = newCanBePressed;
    emitChanged(AbsoluteRangePropName, QVariant());
    emit mObject->settingsChanged(mObject);

    checkStateValidForLock();
}

bool ButtonInterface::canBeExtracted() const
{
    return mCanBeExtracted;
}

void ButtonInterface::setCanBeExtracted(bool newCanBeExtracted)
{
    if(mCanBeExtracted == newCanBeExtracted)
        return;

    if(!mCanBePressed && !newCanBeExtracted)
        return; // At least one must be on

    mCanBeExtracted = newCanBeExtracted;
    emitChanged(AbsoluteRangePropName, QVariant());
    emit mObject->settingsChanged(mObject);

    checkStateValidForLock();
}

void ButtonInterface::setAllowedLockPositions(const std::pair<State, State> &newRange)
{
    if(newRange == mAllowedLockPositions)
        return;

    mAllowedLockPositions = newRange;
    mAllowedLockPositions.second = std::max(mAllowedLockPositions.first,
                                            mAllowedLockPositions.second);

    emitChanged(LockRangePropName, QVariant());
    emit mObject->settingsChanged(mObject);

    checkStateValidForLock();
}

std::pair<ButtonInterface::State, ButtonInterface::State> ButtonInterface::allowedLockPositions() const
{
    auto range = mAllowedLockPositions;
    if(!mCanBePressed)
        range.first = std::max(State::Normal, range.first);
    if(!mCanBeExtracted)
        range.second = std::min(State::Normal, range.second);
    return range;
}
