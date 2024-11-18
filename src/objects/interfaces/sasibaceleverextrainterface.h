#ifndef SASIBACELEVEREXTRAINTERFACE_H
#define SASIBACELEVEREXTRAINTERFACE_H

#include "abstractobjectinterface.h"

class SasibACELeverExtraInterface : public AbstractObjectInterface
{
public:
    SasibACELeverExtraInterface(AbstractSimulationObject *obj);

    static constexpr QLatin1String IfaceType = QLatin1String("sasib_lever_extra");
    QString ifaceType() override;

    // TODO: Tl and Tf buttons
};

#endif // SASIBACELEVEREXTRAINTERFACE_H
