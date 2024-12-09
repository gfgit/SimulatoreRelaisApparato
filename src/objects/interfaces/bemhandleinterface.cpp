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
    if(twinHandle == newTwinHandle)
        return;

    if(twinHandle)
        twinHandle->setTwinHandle(nullptr);

    twinHandle = newTwinHandle;

    if(twinHandle)
    {
        if(twinHandle->leverType() == this->leverType())
        {
            // Twin levers always have opposite types
            LeverType twinType = leverType() == LeverType::Request ?
                        LeverType::Consensus :
                        LeverType::Request;
            twinHandle->setLeverType(twinType);

            twinHandle->setTwinHandle(this);
        }
    }

    emitChanged(TwinLeverPropName, QVariant());
}
