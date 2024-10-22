#include "lightbulbnode.h"

LightBulbNode::LightBulbNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));
}

LightBulbNode::~LightBulbNode()
{

}

QVector<CableItem> LightBulbNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 || !mContacts.at(0).cable)
        return {};

    // Close the circuit
    CableItem dest = source;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

QString LightBulbNode::nodeType() const
{
    return NodeType;
}
