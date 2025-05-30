/**
 * src/objects/simple_activable/lightbulbobject.h
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

#ifndef LIGHTBULBOBJECT_H
#define LIGHTBULBOBJECT_H

#include "abstractsimpleactivableobject.h"

class LightBulbObject : public AbstractSimpleActivableObject
{
    Q_OBJECT
public:
    explicit LightBulbObject(AbstractSimulationObjectModel *m);

    static constexpr QLatin1String Type = QLatin1String("light_bulb");
    QString getType() const override;

    bool setReplicaState(const QCborMap& replicaState) override;
    void getReplicaState(QCborMap& replicaState) const override;

    State state() const override;

protected:
    void onReplicaModeChanged(bool on) override;

private:
    State mReplicaState = State::Off;
};

#endif // LIGHTBULBOBJECT_H
