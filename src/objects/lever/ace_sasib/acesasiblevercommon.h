/**
 * src/objects/lever/ace_sasib/acesasiblevercommon.h
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

#ifndef ACE_SASIB_LEVER_COMMON_H
#define ACE_SASIB_LEVER_COMMON_H

#include "../../abstractsimulationobject.h"

class MechanicalInterface;
class LeverInterface;

struct EnumDesc;
class LeverAngleDesc;

class ElectroMagnetObject;

class ACESasibLeverCommonObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    explicit ACESasibLeverCommonObject(AbstractSimulationObjectModel *m,
                                       const EnumDesc& positionDesc,
                                       const LeverAngleDesc& angleDesc);
    ~ACESasibLeverCommonObject();

    bool loadFromJSON(const QJsonObject& obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject& obj) const override;

    ElectroMagnetObject *magnet() const;
    void setMagnet(ElectroMagnetObject *newMagnet);

private slots:
    void updateElectroMagnetState();

protected:
    void onInterfaceChanged(AbstractObjectInterface *iface,
                            const QString &propName,
                            const QVariant& value) override;

    virtual void addElectromagnetLock() = 0;
    void removeElectromagnetLock();

    void recalculateLockedRange();
    void setNewLockRange();

protected:
    MechanicalInterface *mechanicalIface = nullptr;
    LeverInterface *leverInterface = nullptr;

    ElectroMagnetObject *mMagnet = nullptr;
};

#endif // ACE_SASIB_LEVER_COMMON_H
