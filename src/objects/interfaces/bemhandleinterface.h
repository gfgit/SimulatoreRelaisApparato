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
