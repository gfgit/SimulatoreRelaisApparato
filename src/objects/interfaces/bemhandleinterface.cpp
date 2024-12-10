/**
 * src/objects/interfaces/bemhandleinterface.cpp
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

#include "bemhandleinterface.h"

#include "../abstractsimulationobject.h"
#include "../abstractsimulationobjectmodel.h"

#include "../../utils/enum_desc.h"

#include <QJsonObject>

static const EnumDesc bem_lever_type_desc =
{
    int(BEMHandleInterface::LeverType::Consensus),
    int(BEMHandleInterface::LeverType::Request),
    int(BEMHandleInterface::LeverType::Consensus),
    "BEMLeverObject",
    {
        QT_TRANSLATE_NOOP("BEMLeverObject", "Consensus"),
        QT_TRANSLATE_NOOP("BEMLeverObject", "Request")
    }
};

BEMHandleInterface::BEMHandleInterface(AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
{

}

QString BEMHandleInterface::ifaceType()
{
    return IfaceType;
}

bool BEMHandleInterface::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractObjectInterface::loadFromJSON(obj, phase))
        return false;

    if(phase == LoadPhase::Creation)
    {
        // Type
        LeverType t = LeverType(obj.value("lever_type").toInt());
        setLeverType(t);
    }
    else
    {
        // All objects created, we can build relationships

        const QString twinName = obj.value("twin_handle").toString();
        AbstractSimulationObject *twin = object()->model()->getObjectByName(twinName);
        setTwinHandle(twin ?
                          twin->getInterface<BEMHandleInterface>() :
                          nullptr);
    }

    return true;
}

void BEMHandleInterface::saveToJSON(QJsonObject &obj) const
{
    AbstractObjectInterface::saveToJSON(obj);

    // Type
    obj["lever_type"] = int(mLeverType);

    // Twin
    obj["twin_handle"] = twinHandle ? twinHandle->object()->name() : QString();
}

BEMHandleInterface::LeverType BEMHandleInterface::leverType() const
{
    return mLeverType;
}

void BEMHandleInterface::setLeverType(LeverType newLeverType)
{
    if(mLeverType == newLeverType)
        return;

    mLeverType = newLeverType;

    if(twinHandle)
    {
        // Twin levers always have opposite types
        LeverType twinType = leverType() == LeverType::Request ?
                    LeverType::Consensus :
                    LeverType::Request;
        twinHandle->mLeverType = twinType;
        twinHandle->emitChanged(LeverTypePropName, int(twinHandle->mLeverType));
    }

    emitChanged(LeverTypePropName, int(mLeverType));
}

const EnumDesc &BEMHandleInterface::getLeverTypeDesc()
{
    return bem_lever_type_desc;
}

BEMHandleInterface *BEMHandleInterface::getTwinHandle() const
{
    return twinHandle;
}

void BEMHandleInterface::setTwinHandle(BEMHandleInterface *newTwinHandle)
{
    if(twinHandle == newTwinHandle || twinHandle == this)
        return;

    if(twinHandle)
    {
        twinHandle->twinHandle = nullptr;
        twinHandle->emitChanged(TwinLeverPropName, QVariant());
        twinHandle = nullptr;
    }

    twinHandle = newTwinHandle;

    if(twinHandle)
    {
        if(twinHandle->twinHandle)
        {
            twinHandle->twinHandle->twinHandle = nullptr;
            twinHandle->twinHandle->emitChanged(TwinLeverPropName, QVariant());
            twinHandle->twinHandle = nullptr;
        }

        if(twinHandle->leverType() == this->leverType())
        {
            // Twin levers always have opposite types
            LeverType twinType = leverType() == LeverType::Request ?
                        LeverType::Consensus :
                        LeverType::Request;
            twinHandle->setLeverType(twinType);
        }

        twinHandle->twinHandle = this;
        twinHandle->emitChanged(TwinLeverPropName, QVariant());
    }

    emitChanged(TwinLeverPropName, QVariant());
}
