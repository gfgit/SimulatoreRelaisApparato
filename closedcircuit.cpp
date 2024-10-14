#include "closedcircuit.h"

#include "nodes/powersourcenode.h"

ClosedCircuit::ClosedCircuit(QObject *parent)
    : QObject{parent}
{

}

void ClosedCircuit::enableCircuit()
{
    Q_ASSERT(!enabled);

    for(const Item& item : qAsConst(mItems))
    {
        if(item.isNode)
        {
            item.node->node->addCircuit(this);
        }
        else
        {
            item.cable->cable->addCircuit(this, item.cable->fromSide);
        }
    }

    enabled = true;
}

void ClosedCircuit::disableCircuit()
{
    Q_ASSERT(enabled);

    for(const Item& item : qAsConst(mItems))
    {
        if(item.isNode)
        {
            item.node->node->removeCircuit(this);
        }
        else
        {
            item.cable->cable->removeCircuit(this);
        }
    }

    enabled = false;
}

ClosedCircuit::createCircuitsFromPowerNode(PowerSourceNode *source)
{
    auto side = source->getContacts().first();

    Item firstItem;
    firstItem.isNode = true;
    firstItem.node->node = source;

    for(AbstractCircuitNode::CableItem nodeCable : side.cables)
    {
        firstItem.node->toContact = nodeCable.nodeContact;

        Item item;
        item.cable->cable = nodeCable.cable;
        item.cable->fromSide = nodeCable.cableSide;

        QVector<Item> items;
        items.append(firstItem);
        items.append(item);

        CircuitCable::Side otherSide = CircuitCable::oppositeSide(nodeCable.cableSide);
        CircuitCable::CableEnd cableEnd = nodeCable.cable->getNode(otherSide);
        passCircuitNode(cableEnd.node, cableEnd.nodeContact, items, 0);
    }
}

bool containsNode(const QVector<ClosedCircuit::Item> &items, AbstractCircuitNode *node, int nodeContact)
{
    for(auto item : items)
    {
        if(!item.isNode)
            continue;

        if(item.node->node != node)
            continue;

        if(item.node->fromContact == nodeContact || item.node->toContact == nodeContact)
            return true;
    }

    return false;
}

ClosedCircuit::passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth)
{
    if(depth > 1000)
        return; // TODO

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node->node = node;
    nodeItem.node->fromContact = nodeContact;

    if(node == items.first().node->node)
    {
        // We closed the circuit
        ClosedCircuit *circuit = new ClosedCircuit(node);
        circuit->mItems = items;
        circuit->mItems.append(nodeItem);
        circuit->enableCircuit();
        return;
    }

    CableItem lastCable = items.constLast().cable;

    AbstractCircuitNode::CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable.cable;
    nodeSourceCable.cableSide = CircuitCable::oppositeSide(lastCable.fromSide);
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getConnections(nodeSourceCable);

    for(const auto& conn : connections)
    {
        nodeItem.node->toContact = conn.nodeContact;

        auto otherSide = CircuitCable::oppositeSide(conn.cableSide);
        CircuitCable::CableEnd cableEnd = conn.cable->getNode(otherSide);

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(items, node, nodeContact))
            continue;

        QVector<Item> newItems = items;
        newItems.append(nodeItem);

        passCircuitNode(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }
}
