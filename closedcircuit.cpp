#include "closedcircuit.h"

#include "nodes/powersourcenode.h"

#include <QSet>

ClosedCircuit::ClosedCircuit(QObject *parent)
    : QObject{parent}
{

}

void ClosedCircuit::enableCircuit()
{
    Q_ASSERT(!enabled);

    // First item equal to last, add only last
    for(int i = 1; i < mItems.size(); i++)
    {
        const Item& item = mItems[i];
        if(item.isNode)
        {
            item.node.node->addCircuit(this);
        }
        else
        {
            item.cable.cable->addCircuit(this, item.cable.fromSide);
        }
    }

    enabled = true;
}

void ClosedCircuit::disableCircuit()
{
    Q_ASSERT(enabled);

    // Remove only once per item
    // This way we can assert inside removeCircuit()
    QSet<AbstractCircuitNode *> nodes;
    QSet<CircuitCable *> cables;

    for(const Item& item : std::as_const(mItems))
    {
        if(item.isNode)
        {
            if(!nodes.contains(item.node.node))
            {
                item.node.node->removeCircuit(this);
                nodes.insert(item.node.node);
            }
        }
        else
        {
            if(!cables.contains(item.cable.cable))
            {
                item.cable.cable->removeCircuit(this);
                cables.insert(item.cable.cable);
            }
        }
    }

    enabled = false;
}

void ClosedCircuit::createCircuitsFromPowerNode(PowerSourceNode *source)
{
    auto contact = source->getContacts().first();

    Item firstItem;
    firstItem.isNode = true;
    firstItem.node.node = source;

    for(AbstractCircuitNode::CableItem nodeCable : contact.cables)
    {
        firstItem.node.toContact = nodeCable.nodeContact;

        Item item;
        item.cable.cable = nodeCable.cable;
        item.cable.fromSide = nodeCable.cableSide;

        QVector<Item> items;
        items.append(firstItem);
        items.append(item);

        CircuitCable::Side otherSide = CircuitCable::oppositeSide(nodeCable.cableSide);
        CircuitCable::CableEnd cableEnd = nodeCable.cable->getNode(otherSide);

        if(!cableEnd.node)
            continue;

        passCircuitNode(cableEnd.node, cableEnd.nodeContact, items, 0);
    }
}

bool containsNode(const QVector<ClosedCircuit::Item> &items, AbstractCircuitNode *node, int nodeContact)
{
    for(auto item : items)
    {
        if(!item.isNode)
            continue;

        if(item.node.node != node)
            continue;

        if(item.node.fromContact == nodeContact || item.node.toContact == nodeContact)
            return true;
    }

    return false;
}

void ClosedCircuit::passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth)
{
    if(depth > 1000)
        return; // TODO

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = node;
    nodeItem.node.fromContact = nodeContact;

    if(node == items.first().node.node)
    {
        if(nodeContact != items.first().node.toContact)
        {
            // We closed the circuit
            ClosedCircuit *circuit = new ClosedCircuit(node);
            circuit->mItems = items;
            circuit->mItems.append(nodeItem);
            circuit->enableCircuit();
        }
        return;
    }

    if(qobject_cast<PowerSourceNode *>(node))
    {
        // Error, different power source connected
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
        nodeItem.node.toContact = conn.nodeContact;

        auto otherSide = CircuitCable::oppositeSide(conn.cableSide);
        CircuitCable::CableEnd cableEnd = conn.cable->getNode(otherSide);

        if(!cableEnd.node)
            continue;

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(items, node, nodeContact))
            continue;

        QVector<Item> newItems = items;

        // Append node only if circuit goes through it
        if(nodeItem.node.toContact != nodeItem.node.fromContact)
            newItems.append(nodeItem);

        Item item;
        item.cable.cable = conn.cable;
        item.cable.fromSide = conn.cableSide;
        newItems.append(item);

        passCircuitNode(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }
}

void ClosedCircuit::createCircuitsFromOtherNode(AbstractCircuitNode *source, const QVector<AbstractCircuitNode::NodeContact>& contacts)
{
    for(const auto& contact : contacts)
    {
        for(AbstractCircuitNode::CableItem nodeCable : contact.cables)
        {
            CircuitCable::Side otherSide = CircuitCable::oppositeSide(nodeCable.cableSide);

            Item item;
            item.cable.cable = nodeCable.cable;
            item.cable.fromSide = otherSide;

            QVector<Item> items;
            items.append(item);

            CircuitCable::CableEnd cableEnd = nodeCable.cable->getNode(otherSide);

            if(!cableEnd.node)
                continue;

            searchPowerSource(cableEnd.node, cableEnd.nodeContact, items, 0);
        }
    }
}

void ClosedCircuit::searchPowerSource(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth)
{
    if(depth > 1000)
        return; // TODO

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = node;
    nodeItem.node.toContact = nodeContact; // Backwards

    if(PowerSourceNode *powerSource = qobject_cast<PowerSourceNode *>(node))
    {
        // Make circuits start from positive source contact (0)
        // Do not add circuits to disabled power sources
        if(!powerSource->getEnabled() || nodeContact != 0)
            return;

        // We got a power source!
        // Reverse item order, then finish building circuits
        QVector<Item> realItems = items;
        realItems.append(nodeItem);
        std::reverse(realItems.begin(), realItems.end());

        continueCircuitPassingLastNode(realItems, depth);

        return;
    }

    CableItem lastCable = items.constLast().cable;

    AbstractCircuitNode::CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable.cable;
    nodeSourceCable.cableSide = lastCable.fromSide; // Backwards
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getConnections(nodeSourceCable);

    for(const auto& conn : connections)
    {
        nodeItem.node.fromContact = conn.nodeContact;

        auto otherSide = CircuitCable::oppositeSide(conn.cableSide);
        CircuitCable::CableEnd cableEnd = conn.cable->getNode(otherSide);

        if(!cableEnd.node)
            continue;

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(items, node, nodeContact))
            continue;

        QVector<Item> newItems = items;

        // Append node only if circuit goes through it
        if(nodeItem.node.toContact != nodeItem.node.fromContact)
            newItems.append(nodeItem);

        Item item;
        item.cable.cable = conn.cable;
        item.cable.fromSide = otherSide; // Backwards
        newItems.append(item);

        searchPowerSource(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }
}

void ClosedCircuit::continueCircuitPassingLastNode(const QVector<Item> &items, int depth)
{
    CableItem lastCable = items.constLast().cable;
    CircuitCable::Side otherSide = CircuitCable::oppositeSide(lastCable.fromSide);
    CircuitCable::CableEnd lastCableEnd = lastCable.cable->getNode(otherSide);

    AbstractCircuitNode::CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable.cable;
    nodeSourceCable.cableSide = CircuitCable::oppositeSide(lastCable.fromSide);
    nodeSourceCable.nodeContact = lastCableEnd.nodeContact;
    const auto connections = lastCableEnd.node->getConnections(nodeSourceCable);

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = lastCableEnd.node;
    nodeItem.node.fromContact = lastCableEnd.nodeContact;

    for(const auto& conn : connections)
    {
        nodeItem.node.toContact = conn.nodeContact;
        if(nodeItem.node.toContact == nodeItem.node.fromContact)
            continue; // Skip circuits not passing through this node

        auto otherSide = CircuitCable::oppositeSide(conn.cableSide);
        CircuitCable::CableEnd cableEnd = conn.cable->getNode(otherSide);

        if(!cableEnd.node)
            continue;

        if(cableEnd.node == nodeItem.node.node && cableEnd.nodeContact == nodeItem.node.fromContact)
        {
            continue;
        }

        if(containsNode(items, cableEnd.node, cableEnd.nodeContact))
            continue;

        QVector<Item> newItems = items;

        // Append node
        newItems.append(nodeItem);

        Item item;
        item.cable.cable = conn.cable;
        item.cable.fromSide = conn.cableSide;
        newItems.append(item);

        passCircuitNode(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }

    return;
}
