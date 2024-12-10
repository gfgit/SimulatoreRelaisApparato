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

#include <QJsonObject>

static const EnumDesc button_mode_desc =
{
    int(ButtonInterface::Mode::AutoReturnNormal),
    int(ButtonInterface::Mode::ReturnNormalAfterTimout),
    int(ButtonInterface::Mode::AutoReturnNormal),
    "GenericButtonObject",
    {
        QT_TRANSLATE_NOOP("GenericButtonObject", "Auto Return Normal"),
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

QVector<AbstractCircuitNode *> ButtonInterface::nodes() const
{
    QVector<AbstractCircuitNode *> result;
    result.reserve(mContactNodes.size());
    for(auto item : mContactNodes)
        result.append(item);
    return result;
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

    setMode(Mode(obj.value("mode").toInt(int(Mode::AutoReturnNormal))));

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
    if(mState == newState)
        return;

    if(!mCanBePressed && newState == State::Pressed)
        return;
    if(!mCanBeExtracted && newState == State::Extracted)
        return;

    mState = newState;

    emitChanged(StatePropName, int(mState));
    emit mObject->stateChanged(mObject);
}

void ButtonInterface::addContactNode(ButtonContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));

    mContactNodes.append(c);

    emit mObject->nodesChanged();
}

void ButtonInterface::removeContactNode(ButtonContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->button() == mObject);

    mContactNodes.removeOne(c);

    emit mObject->nodesChanged();
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
    emit mObject->stateChanged(mObject);
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
    emit mObject->settingsChanged(mObject);

    if(!mCanBePressed && state() == State::Pressed)
        setState(State::Normal);
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
    emit mObject->settingsChanged(mObject);

    if(!mCanBeExtracted && state() == State::Extracted)
        setState(State::Normal);
}
