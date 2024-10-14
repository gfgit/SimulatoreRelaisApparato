#include "simplecircuitnode.h"

SimpleCircuitNode::SimpleCircuitNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 2 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<AbstractCircuitNode::CableItem> SimpleCircuitNode::getConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 || source.nodeContact != 1)
        return {};

    const NodeContact& side = mContacts[source.nodeContact];

    QVector<AbstractCircuitNode::CableItem> result;
    result.reserve(side.cables.size());

    for(const CableItem& item : side.cables)
    {
        if(item == source)
            continue;

        result.append(item);
    }

    return result;
}
