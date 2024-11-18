#include "sasibaceleverextrainterface.h"

SasibACELeverExtraInterface::SasibACELeverExtraInterface(AbstractSimulationObject *obj)
    : AbstractObjectInterface(obj)
{

}

QString SasibACELeverExtraInterface::ifaceType()
{
    return IfaceType;
}
