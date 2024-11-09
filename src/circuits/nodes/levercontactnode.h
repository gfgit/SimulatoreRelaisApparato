/**
 * src/circuits/nodes/levercontactnode.h
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

#ifndef LEVERCONTACTNODE_H
#define LEVERCONTACTNODE_H

#include "abstractdeviatornode.h"

#include "../../enums/genericleverposition.h"

class GenericLeverObject;

class LeverContactNode : public AbstractDeviatorNode
{
    Q_OBJECT
public:
    enum State
    {
        Down = 0,
        Up = 1,
        Middle
    };

    explicit LeverContactNode(ModeManager *mgr, QObject *parent = nullptr);
    ~LeverContactNode();

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("lever_contact");
    QString nodeType() const override;

    GenericLeverObject *lever() const;
    void setLever(GenericLeverObject *newLever);

    State state() const;
    void setState(State newState);

    LeverPositionConditionSet conditionSet() const;
    void setConditionSet(const LeverPositionConditionSet &newConditionSet);

    bool isPositionOn(int pos) const;

signals:
    void leverChanged(GenericLeverObject *l);

private slots:
    void onLeverPositionChanged();

private:
    GenericLeverObject *mLever = nullptr;

    LeverPositionConditionSet mConditionSet;

    State mState = State::Middle;
};

#endif // LEVERCONTACTNODE_H
