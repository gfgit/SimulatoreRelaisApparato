/**
 * src/objects/interfaces/buttoninterface.h
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

#ifndef BUTTONINTERFACE_H
#define BUTTONINTERFACE_H

#include "abstractobjectinterface.h"

#include <QVector>

class ButtonContactNode;

class ButtonInterface : public AbstractObjectInterface
{
public:
    // Property names
    static constexpr QLatin1String StatePropName = QLatin1String("state");

    enum class State
    {
        Normal = 0,
        Pressed,
        Extracted
    };

    ButtonInterface(AbstractSimulationObject *obj);
    ~ButtonInterface();

    static constexpr QLatin1String IfaceType = QLatin1String("button");
    QString ifaceType() override;

    QVector<AbstractCircuitNode*> nodes() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    // State
    State state() const;
    void setState(State newState);

    // Options


    bool canBePressed() const;
    void setCanBePressed(bool newCanBePressed);

    bool canBeExtracted() const;
    void setCanBeExtracted(bool newCanBeExtracted);

private:
    friend class ButtonContactNode;
    void addContactNode(ButtonContactNode *c);
    void removeContactNode(ButtonContactNode *c);

private:
    State mState = State::Normal;

    bool mCanBePressed = true;
    bool mCanBeExtracted = false;

    QVector<ButtonContactNode *> mContactNodes;
};

#endif // BUTTONINTERFACE_H