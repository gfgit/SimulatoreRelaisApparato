/**
 * src/objects/interfaces/bemhandleinterface.h
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

#ifndef BEMHANDLEINTERFACE_H
#define BEMHANDLEINTERFACE_H

#include "abstractobjectinterface.h"

class EnumDesc;

class BEMHandleInterface : public AbstractObjectInterface
{
public:
    // Property names
    static constexpr QLatin1String LeverTypePropName = QLatin1String("lever_type");
    static constexpr QLatin1String TwinLeverPropName = QLatin1String("twin_lever");

    enum LeverType
    {
        Consensus = 0,
        Request = 1
    };

    BEMHandleInterface(AbstractSimulationObject *obj);

    static constexpr QLatin1String IfaceType = QLatin1String("bem_lever");
    QString ifaceType() override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    // Options
    LeverType leverType() const;
    void setLeverType(LeverType newLeverType);

    static const EnumDesc& getLeverTypeDesc();

    BEMHandleInterface *getTwinHandle() const;
    void setTwinHandle(BEMHandleInterface *newTwinHandle);

private:
    LeverType mLeverType = LeverType::Consensus;

    BEMHandleInterface *twinHandle = nullptr;
};

#endif // BEMHANDLEINTERFACE_H
