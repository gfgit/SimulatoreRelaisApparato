#ifndef STANDARDNODETYPES_H
#define STANDARDNODETYPES_H

#include "nodeeditfactory.h"

#include <QCoreApplication>

class StandardNodeTypes
{
    Q_DECLARE_TR_FUNCTIONS(StandardNodeTypes)

public:
    static void registerTypes(NodeEditFactory *factoryReg);
};

#endif // STANDARDNODETYPES_H
