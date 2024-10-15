#include "abstractnodegraphitem.h"

#include "../abstractcircuitnode.h"

AbstractNodeGraphItem::AbstractNodeGraphItem(AbstractCircuitNode *node_)
    : QGraphicsObject()
    , mAbstractNode(node_)
{
    setParent(mAbstractNode);

    connect(mAbstractNode, &QObject::objectNameChanged,
            this, &AbstractNodeGraphItem::updateName);

    updateName();
}

void AbstractNodeGraphItem::triggerUpdate()
{
    update();
}

void AbstractNodeGraphItem::updateName()
{
    setToolTip(mAbstractNode->objectName());
}
