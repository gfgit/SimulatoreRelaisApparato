/**
 * src/objects/button/genericbuttonobject.h
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

#ifndef GENERICBUTTONOBJECT_H
#define GENERICBUTTONOBJECT_H

#include "../abstractsimulationobject.h"

#include <QBasicTimer>

class ButtonInterface;
class MechanicalInterface;

class GenericButtonObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    explicit GenericButtonObject(AbstractSimulationObjectModel *m);
    ~GenericButtonObject();

    static constexpr QLatin1String Type = QLatin1String("generic_button");
    QString getType() const override;

    bool setReplicaState(const QCborMap& replicaState) override;
    void getReplicaState(QCborMap& replicaState) const override;

protected:
    void timerEvent(QTimerEvent *ev) override;

    void onInterfaceChanged(AbstractObjectInterface *iface,
                            const QString &propName,
                            const QVariant &value) override;

    void onReplicaModeChanged(bool on) override;

private:
    void startReturnTimer();

    void setNewLockRange();

private:
    ButtonInterface *buttonIface = nullptr;
    MechanicalInterface *mechanicalIface = nullptr;

    QBasicTimer mReturnTimeout;
    int mReturnTimerId = 0;
};

#endif // GENERICBUTTONOBJECT_H
