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
            item.cable.cable->addCircuit(this, item.cable.pole);
        }
    }

    enabled = true;
}

void ClosedCircuit::disableCircuit()
{
    Q_ASSERT(enabled);

    // Guard against recursive disabling of circuit
    if(isDisabling)
        return;

    isDisabling = true;

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

    isDisabling = false;

    enabled = false;
}

QVector<ClosedCircuit::NodeItem> ClosedCircuit::getNode(AbstractCircuitNode *node) const
{
    QVector<ClosedCircuit::NodeItem> result;

    for(const Item& item : std::as_const(mItems))
    {
        if(item.isNode && item.node.node == node)
            result.append(item.node);
    }
    return result;
}

void ClosedCircuit::createCircuitsFromPowerNode(PowerSourceNode *source)
{
    auto contact = source->getContacts().first();

    Item firstItem;
    firstItem.isNode = true;
    firstItem.node.node = source;
    firstItem.node.fromPole = CircuitCable::Pole::First;

    if(contact.cable)
    {
        firstItem.node.toContact = 0; // First
        firstItem.node.toPole = CircuitCable::Pole::First;

        Item item;
        item.cable.cable = contact.cable;
        item.cable.side = contact.cableSide;

        QVector<Item> items;
        items.append(firstItem);
        items.append(item);

        CircuitCable::Side otherSide = ~contact.cableSide;
        CircuitCable::CableEnd cableEnd = contact.cable->getNode(otherSide);

        if(!cableEnd.node)
            return;

        passCircuitNode(cableEnd.node, cableEnd.nodeContact, items, 0);
    }
}

bool containsNode(const QVector<ClosedCircuit::Item> &items, AbstractCircuitNode *node, int nodeContact, CircuitCable::Pole pole)
{
    for(auto item : items)
    {
        if(!item.isNode)
            continue;

        if(item.node.node != node)
            continue;

        if(item.node.fromContact == nodeContact && item.node.fromPole == pole)
            return true;

        if(item.node.toContact == nodeContact && item.node.toPole == pole)
            return true;
    }

    return false;
}

void ClosedCircuit::passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth)
{
    if(depth > 1000)
        return; // TODO

    CircuitCable::CableContact lastCable = items.constLast().cable;

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = node;
    nodeItem.node.fromContact = nodeContact;
    nodeItem.node.fromPole = lastCable.pole;

    if(node == items.first().node.node)
    {
        if(nodeItem.node.fromPole == CircuitCable::Pole::Second)
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

    AbstractCircuitNode::CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable;
    nodeSourceCable.cable.side = ~lastCable.side;
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getActiveConnections(nodeSourceCable);

    for(const auto& conn : connections)
    {
        if(!conn.cable.cable)
            continue;

        nodeItem.node.toContact = conn.nodeContact;
        nodeItem.node.toPole = conn.cable.pole;

        // Get opposite side
        CircuitCable::CableEnd cableEnd = conn.cable.cable->getNode(~conn.cable.side);

        if(!cableEnd.node)
            continue;

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(items, node, nodeContact, lastCable.pole))
            continue;

        QVector<Item> newItems = items;
        newItems.append(nodeItem);

        Item item;
        item.cable = conn.cable;
        newItems.append(item);

        passCircuitNode(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }
}

void ClosedCircuit::createCircuitsFromOtherNode(AbstractCircuitNode *source, const QVector<AbstractCircuitNode::NodeContact>& contacts)
{
    for(const auto& contact : contacts)
    {
        if(!contact.cable)
            continue;

        // Get opposite side
        CircuitCable::CableEnd cableEnd = contact.cable->getNode(~contact.cableSide);

        if(!cableEnd.node)
            continue;

        Item item;
        item.cable.cable = contact.cable;
        item.cable.side = ~contact.cableSide;
        item.cable.pole = CircuitCable::Pole::First;

        QVector<Item> items;
        items.append(item);

        searchPowerSource(cableEnd.node, cableEnd.nodeContact, items, 0);

        items.first().cable.pole = CircuitCable::Pole::Second;
        searchPowerSource(cableEnd.node, cableEnd.nodeContact, items, 0);
    }
}

void ClosedCircuit::searchPowerSource(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth)
{
    if(depth > 1000)
        return; // TODO

    CircuitCable::CableContact lastCable = items.constLast().cable;

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = node;
    nodeItem.node.toContact = nodeContact; // Backwards
    nodeItem.node.toPole = lastCable.pole;

    if(PowerSourceNode *powerSource = qobject_cast<PowerSourceNode *>(node))
    {
        // Make circuits start from positive source pole (First pole)
        // Do not add circuits to disabled power sources
        if(!powerSource->getEnabled() || nodeItem.node.toPole != CircuitCable::Pole::First)
            return;

        // We got a power source!
        // Reverse item order, then finish building circuits
        QVector<Item> realItems = items;
        realItems.append(nodeItem);
        std::reverse(realItems.begin(), realItems.end());

        continueCircuitPassingLastNode(realItems, depth);

        return;
    }

    // Go backwards
    AbstractCircuitNode::CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable; // Backwards
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getActiveConnections(nodeSourceCable, true);

    for(const auto& conn : connections)
    {
        nodeItem.node.fromContact = conn.nodeContact;
        nodeItem.node.fromPole = conn.cable.pole;

        auto cableEnd = conn.cable.cable->getNode(~conn.cable.side);

        if(!cableEnd.node)
            continue;

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(items, node, nodeContact, lastCable.pole))
            continue;

        QVector<Item> newItems = items;
        newItems.append(nodeItem);

        Item item;
        item.cable = conn.cable;
        item.cable.side = ~conn.cable.side; // Backwards
        newItems.append(item);

        searchPowerSource(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }
}

void ClosedCircuit::continueCircuitPassingLastNode(const QVector<Item> &items, int depth)
{
    CircuitCable::CableContact lastCable = items.constLast().cable;

    // Get opposite side
    CircuitCable::CableEnd lastCableEnd = lastCable.cable->getNode(~lastCable.side);

    AbstractCircuitNode::CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable;
    nodeSourceCable.cable.side = ~lastCable.side;
    nodeSourceCable.nodeContact = lastCableEnd.nodeContact;
    const auto connections = lastCableEnd.node->getActiveConnections(nodeSourceCable);

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = lastCableEnd.node;
    nodeItem.node.fromContact = lastCableEnd.nodeContact;
    nodeItem.node.fromPole = lastCable.pole;

    for(const auto& conn : connections)
    {
        nodeItem.node.toContact = conn.nodeContact;
        nodeItem.node.toPole = conn.cable.pole;

        // Get opposite cable side
        auto cableEnd = conn.cable.cable->getNode(~conn.cable.side);

        if(!cableEnd.node)
            continue;

        if(cableEnd.node == nodeItem.node.node && cableEnd.nodeContact == nodeItem.node.fromContact)
        {
            continue;
        }

        if(containsNode(items, cableEnd.node, cableEnd.nodeContact, lastCable.pole))
            continue;

        QVector<Item> newItems = items;

        // Append node
        newItems.append(nodeItem);

        Item item;
        item.cable = conn.cable;
        newItems.append(item);

        passCircuitNode(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }

    return;
}
