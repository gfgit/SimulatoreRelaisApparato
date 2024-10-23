#include "../core/electriccircuit.h"

#include "../nodes/circuitcable.h"
#include "../nodes/powersourcenode.h"

#include <QSet>

ElectricCircuit::ElectricCircuit()
{

}

void ElectricCircuit::enableCircuit()
{
    Q_ASSERT(!enabled);

    for(int i = 0; i < mItems.size(); i++)
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

bool ElectricCircuit::tryReachOpen(AbstractCircuitNode *goalNode)
{
    Q_ASSERT(type() == CircuitType::Closed);
    Q_ASSERT(!enabled);

    // Trasform this circuit in an open circuit
    mType = CircuitType::Open;

    if(mItems.isEmpty())
        return false;

    if(goalNode == getSource())
        return false; // Empty circuit

    CableItem nodeSourceCable;

    int nodeIdx = -1;

    // Ignore power source
    for(int i = 1; i < mItems.size(); i++)
    {
        const Item& item = mItems[i];
        if(item.isNode)
        {
            if(item.node.node == goalNode)
            {
                nodeIdx = i;
                break;
            }

            nodeSourceCable.nodeContact = item.node.fromContact;
            const auto connections = item.node.node->getActiveConnections(nodeSourceCable);

            bool found = false;
            for(const auto &conn : connections)
            {
                if(conn.nodeContact != item.node.toContact)
                    continue;

                if(conn.cable.pole != item.node.toPole)
                    continue;

                found = true;
                break;
            }

            if(!found)
                return false;
        }
        else
        {
            nodeSourceCable.cable = item.cable;
            nodeSourceCable.cable.side = ~item.cable.side;
            if(!nodeSourceCable.cable.cable)
                return false;
        }
    }

    Q_ASSERT(nodeIdx >= 0);
    int firstIdxToRemove = nodeIdx + 1;
    mItems.remove(firstIdxToRemove, mItems.size() - firstIdxToRemove);

    // Remove last node toContact
    mItems.last().node.toContact = NodeItem::InvalidContact;

    enableCircuit();
    return true;
}

void ElectricCircuit::disableOrTerminate(AbstractCircuitNode *node)
{
    Q_ASSERT(type() == CircuitType::Closed);
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
                NodeOccurences items = getNode(item.node.node);
                item.node.node->removeCircuit(this, items);
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

    if(!tryReachOpen(node))
        delete this;
}

void ElectricCircuit::terminateHere(AbstractCircuitNode *goalNode,
                                    QVector<ElectricCircuit *> &deduplacteList)
{
    Q_ASSERT(type() == CircuitType::Open);
    Q_ASSERT(enabled);

    // Remove only once per item
    // This way we can assert inside removeCircuit()
    QSet<AbstractCircuitNode *> nodes;
    QSet<CircuitCable *> cables;

    QSet<AbstractCircuitNode *> nodesToKeep;
    QSet<CircuitCable *> cablesToKeep;

    int nodeIdx = -1;
    for(int i = 0; i < mItems.size(); i++)
    {
        const Item& item = mItems.at(i);
        if(item.isNode)
        {
            nodesToKeep.insert(item.node.node);
            if(item.node.node == goalNode)
            {
                nodeIdx = i;
                break;
            }
        }
        else
        {
            cablesToKeep.insert(item.cable.cable);
        }
    }

    Q_ASSERT(nodeIdx >= 0);

    // If node is first remove all, otherwise remove after node
    int firstIdxToRemove = nodeIdx > 0 ? nodeIdx + 1 : 0;

    if(firstIdxToRemove > 0)
    {
        for(const ElectricCircuit *duplicate : std::as_const(deduplacteList))
        {
            PowerSourceNode *otherSource = duplicate->getSource();
            if(!otherSource || otherSource != getSource())
                continue; // Different sources, cannot be duplicate

            // If same node is in different position, circuits have different path
            if(duplicate->mItems.size() != nodeIdx + 1)
                continue;

            // Possible duplicate, check all items
            bool samePath = true;
            for(int i = 0; i <= nodeIdx; i++)
            {
                Item otherItem = duplicate->mItems.at(i);
                if(mItems.at(i) == otherItem)
                    continue;

                if(i == nodeIdx && otherItem.node.toContact == NodeItem::InvalidContact)
                {
                    // Last node differs because of toContact
                    // Which should be -1 for duplicate circuit
                    // But on our circuit could still be valid

                    // Fake toContact and re-check. Real value will be adjusted later
                    otherItem.node.toContact = mItems.at(i).node.toContact;
                    if(mItems.at(i) == otherItem)
                        continue;
                }

                samePath = false;
                break;
            }

            if(samePath)
            {
                // Circuits are duplicates
                // Remove ourserlves by removing all nodes
                firstIdxToRemove = 0;
                break;
            }
        }
    }

    if(firstIdxToRemove == 0)
    {
        // Do not keep registered on any item
        nodesToKeep.clear();
        cablesToKeep.clear();
    }

    for(int i = firstIdxToRemove; i < mItems.size(); i++)
    {
        const Item& item = mItems.at(i);

        if(item.isNode)
        {
            if(nodesToKeep.contains(item.node.node))
            {
                // Remove only this passage
                item.node.node->partialRemoveCircuit(this, {item.node});
            }
            else if(!nodes.contains(item.node.node))
            {
                // Remove every occurrence of this circuit
                // Then save node to avoid removing again
                NodeOccurences items = getNode(item.node.node);
                item.node.node->removeCircuit(this, items);
                nodes.insert(item.node.node);
            }
        }
        else
        {
            if(!cables.contains(item.cable.cable) && !cablesToKeep.contains(item.cable.cable))
            {
                item.cable.cable->removeCircuit(this);
                cables.insert(item.cable.cable);
            }
        }
    }

    mItems.remove(firstIdxToRemove, mItems.size() - firstIdxToRemove);

    if(mItems.isEmpty())
    {
        // We are an empty circuit
        delete this;
    }
    else
    {
        // Circuit is still registered at node
        // Remove toContact from last node
        goalNode->unregisterOpenCircuitExit(this);
        mItems.last().node.toContact = NodeItem::InvalidContact;

        deduplacteList.append(this);
    }
}

QVector<NodeItem> ElectricCircuit::getNode(AbstractCircuitNode *node) const
{
    QVector<NodeItem> result;

    for(const Item& item : std::as_const(mItems))
    {
        if(item.isNode && item.node.node == node)
            result.append(item.node);
    }
    return result;
}

bool ElectricCircuit::isLastNode(AbstractCircuitNode *node) const
{
    if(mItems.isEmpty())
        return false;

    const Item& item = mItems.last();
    if(item.isNode && item.node.node == node)
        return true;
    return false;
}

PowerSourceNode *ElectricCircuit::getSource() const
{
    if(mItems.isEmpty())
        return nullptr;

    const Item& item = mItems.first();
    if(!item.isNode)
        return nullptr;

    return qobject_cast<PowerSourceNode *>(item.node.node);
}

void ElectricCircuit::createCircuitsFromPowerNode(PowerSourceNode *source)
{
    auto contact = source->getContacts().first();

    Item firstItem;
    firstItem.isNode = true;
    firstItem.node.node = source;
    firstItem.node.fromContact = NodeItem::InvalidContact;
    firstItem.node.fromPole = CircuitPole::First;
    firstItem.node.toContact = 0; // First
    firstItem.node.toPole = CircuitPole::First;

    if(contact.cable)
    {
        Item nextCable;
        nextCable.cable.cable = contact.cable;
        nextCable.cable.side = contact.cableSide;

        QVector<Item> items;
        items.append(firstItem);
        items.append(nextCable);

        CableSide otherSide = ~contact.cableSide;
        CableEnd cableEnd = contact.cable->getNode(otherSide);

        if(!cableEnd.node)
        {
            // Register an open circuit which passes through node
            // And then go to next cable
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = items;
            circuit->mType = CircuitType::Open;
            circuit->enableCircuit();
            return;
        }

        // Depth 1 because we already passed PowerSource node
        passCircuitNode(cableEnd.node, cableEnd.nodeContact, items, 1);
    }
    else
    {
        // Register an open circuit which passes through node
        ElectricCircuit *circuit = new ElectricCircuit();
        circuit->mItems.append(firstItem);
        circuit->mType = CircuitType::Open;
        circuit->enableCircuit();
    }
}

bool containsNode(const QVector<ElectricCircuit::Item> &items, AbstractCircuitNode *node, int nodeContact, CircuitPole pole)
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

bool ElectricCircuit::passCircuitNode(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth)
{
    // Returns true if circuit goes to next node
    if(depth > 1000)
        return false; // TODO

    CableContact lastCable = items.constLast().cable;

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = node;
    nodeItem.node.fromContact = nodeContact;
    nodeItem.node.fromPole = lastCable.pole;
    nodeItem.node.toContact = NodeItem::InvalidContact;

    if(node == items.first().node.node)
    {
        if(nodeItem.node.fromPole == CircuitPole::Second)
        {
            // We closed the circuit
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = items;
            circuit->mItems.append(nodeItem);
            circuit->mType = CircuitType::Closed;
            circuit->enableCircuit();
        }
        return false;
    }

    if(qobject_cast<PowerSourceNode *>(node))
    {
        // Error, different power source connected
        return false;
    }

    CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable;
    nodeSourceCable.cable.side = ~lastCable.side;
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getActiveConnections(nodeSourceCable);

    bool circuitEndsHere = true;

    for(const auto& conn : connections)
    {
        nodeItem.node.toContact = conn.nodeContact;
        nodeItem.node.toPole = conn.cable.pole;

        Item nextCable;
        nextCable.cable = conn.cable;

        if(!conn.cable.cable)
        {
            // Register an open circuit which passes through node
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = items;
            circuit->mItems.append(nodeItem);
            circuit->mType = CircuitType::Open;
            circuit->enableCircuit();

            // Circuit will pass to opposite node connector
            circuitEndsHere = false;
            continue;
        }

        // Get opposite side
        CableEnd cableEnd = conn.cable.cable->getNode(~conn.cable.side);

        if(!cableEnd.node)
        {
            // Register an open circuit which passes through node
            // And then go to next cable
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = items;
            circuit->mItems.append(nodeItem);
            circuit->mItems.append(nextCable);
            circuit->mType = CircuitType::Open;
            circuit->enableCircuit();

            // Circuit will pass to opposite node connector
            circuitEndsHere = false;
            continue;
        }

        // At this point one of the following can happen:
        // 1- Cable is connected both sides to same node (rather impossible)
        // 2- Circuit already contain this node with same contact and polarity
        //    So it's an invalid circuit with infinite loop and must be ignored
        // 3- Circuit will pass node and go on
        // In all cases above we consider circuit as not ending here
        // So that original circuit can be freed
        // This avoid keeping around duplicate Open Circuits
        circuitEndsHere = false;

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(items, node, nodeContact, lastCable.pole))
            continue;

        QVector<Item> newItems = items;
        newItems.append(nodeItem);
        newItems.append(nextCable);
        passCircuitNode(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }

    if(circuitEndsHere)
    {
        if(depth > 0)
        {
            // Register an open circuit which passes HALF node
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = items;
            circuit->mItems.append(nodeItem);
            circuit->mType = CircuitType::Open;
            circuit->enableCircuit();
        }

        return false;
    }

    return true;
}

void ElectricCircuit::createCircuitsFromOtherNode(AbstractCircuitNode *node)
{
    QVector<ElectricCircuit *> openCircuitsCopy = node->getCircuits(CircuitType::Open);

    for(ElectricCircuit *origCircuit : openCircuitsCopy)
    {
        // Try to continue circuit
        Q_ASSERT(origCircuit->mItems.size() >= 3); // PowerSource + cable + Our Node
        Q_ASSERT(origCircuit->mItems.last().isNode);

        int fromContact = origCircuit->mItems.last().node.fromContact;
        QVector<Item> items = origCircuit->mItems;
        items.removeLast(); // Remove node

        // Pass again on our node
        // Depth 0 because if no connection is found to other nodes
        // Do not add a new open circuit which would be equivalent
        // To origCircuit being used here
        if(passCircuitNode(node, fromContact, items, 0))
        {
            // Circuit went ahead, delete old open circuit
            QVector<ElectricCircuit *> dummy;
            origCircuit->terminateHere(origCircuit->getSource(), dummy);
        }
    }
}
