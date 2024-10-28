/**
 * src/core/electriccircuit.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "../core/electriccircuit.h"

#include "../nodes/circuitcable.h"
#include "../nodes/powersourcenode.h"

#include <QSet>
#include <QVarLengthArray>

#include <QDebug>

static int allCircuitsCount = 0;

ElectricCircuit::ElectricCircuit()
{
    allCircuitsCount++;
}

ElectricCircuit::~ElectricCircuit()
{
    // Unregister from parent circuit
    setParent(nullptr);

    // Delete all children
    disableChildren();

    allCircuitsCount--;
}

void ElectricCircuit::enableCircuit()
{
    Q_ASSERT(!enabled);
    Q_ASSERT(!mItems.isEmpty());
    Q_ASSERT(getSource());

    // Check for duplicate circuits
    // TODO: this should not happen
    if(!parent())
    {
        // Toplevel circuit
        PowerSourceNode *source = getSource();
        for(ElectricCircuit *other : source->getCircuits(type()))
        {
            if(other->mItems == mItems)
            {
                // We are a duplicate
                qDebug() << "DUPLICATE CIRCUIT OF TYPE:" << (type() == CircuitType::Closed ? "closed" : "open");
                delete this;
                return;
            }
        }

        // Duplicate of different type. This really should not happen!
        CircuitType otherType = type() == CircuitType::Closed ? CircuitType::Open : CircuitType::Closed;
        for(ElectricCircuit *other : source->getCircuits(otherType))
        {
            if(other->mItems == mItems)
            {
                // We are a duplicate
                qDebug() << "DUPLICATE CIRCUIT OF OPPOSITE TYPE:" << (type() == CircuitType::Closed ? "closed" : "open");
                delete this;
                return;
            }
        }
    }
    else
    {
        for(ElectricCircuit *other : parent()->getChildren())
        {
            if(other == this)
                continue; // Don't compare against ourself

            if(other->mItems == mItems)
            {
                // We are a duplicate
                qDebug() << "DUPLICATE CIRCUIT child:" << (type() == CircuitType::Closed ? "closed" : "open");
                delete this;
                return;
            }
        }
    }


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

    // Guard against recursive disabling of circuit
    if(isDisabling)
        return;

    isDisabling = true;

    disableChildren();
    disableInternal();

    if(type() == CircuitType::Closed && parent() && !parent()->isDisabling)
    {
        // Demote parent from Closed to Open circuit
        if(parent())
        {
            Q_ASSERT(parent()->mClosedChildCount > 0);
            parent()->mClosedChildCount--;

            parent()->promoteToType(CircuitType::Open);
        }
    }

    isDisabling = false;

    if(!tryReachOpen(node))
        delete this;
}

bool ElectricCircuit::terminateHere(AbstractCircuitNode *goalNode,
                                    QVector<ElectricCircuit *> &deduplacteList)
{
    Q_ASSERT(type() == CircuitType::Open);
    Q_ASSERT(enabled);

    disableChildren();

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
        return true;
    }
    else
    {
        // Circuit is still registered at node
        // Remove toContact from last node
        goalNode->unregisterOpenCircuitExit(this);
        mItems.last().node.toContact = NodeItem::InvalidContact;

        deduplacteList.append(this);
    }

    return false;
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
    const ElectricCircuit *circuit = this;

    while(circuit->parent())
        circuit = circuit->parent();

    if(circuit->mItems.isEmpty())
        return nullptr;

    const Item& item = circuit->mItems.first();
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
        // No parent
        passCircuitNode(nullptr, cableEnd.node, cableEnd.nodeContact, items, 1);
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

bool ElectricCircuit::containsNode(const QVector<ElectricCircuit::Item> &items, AbstractCircuitNode *node, int nodeContact, CircuitPole pole)
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

bool ElectricCircuit::containsNode(ElectricCircuit *circuit, AbstractCircuitNode *node, int nodeContact, CircuitPole pole)
{
    bool lastCircuit = true;
    while(circuit)
    {
        if(containsNode(circuit->mItems, node, nodeContact, pole))
        {
            // Last item is duplicated, avoid matching it
            if(lastCircuit && circuit->mItems.size())
            {
                Item otherItem = circuit->mItems.last();
                if(otherItem.isNode &&
                        otherItem.node.node == node &&
                        otherItem.node.toContact == NodeItem::InvalidContact)
                    return false; // Let new circuit passa on this node
            }

            // Duplicate node, circuit would be infinite loop
            return true;
        }
        circuit = circuit->parent();
        lastCircuit = false;
    }

    return false;
}

void ElectricCircuit::passCircuitNode(ElectricCircuit *parent, AbstractCircuitNode *node,
                                      int nodeContact,
                                      const QVector<Item> &items,
                                      int depth, bool extendParent)
{
    // Returns true if circuit goes to next node
    if(depth > 1000)
        return; // TODO

    CableContact lastCable = items.constLast().cable;

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = node;
    nodeItem.node.fromContact = nodeContact;
    nodeItem.node.fromPole = lastCable.pole;
    nodeItem.node.toContact = NodeItem::InvalidContact;

    if(PowerSourceNode *pwNode = qobject_cast<PowerSourceNode *>(node))
    {
        AbstractCircuitNode *source = nullptr;
        if(parent)
            source = parent->getSource();
        else
            source = items.first().node.node;

        if(pwNode != source)
        {
            // Error, different power source connected
            return;
        }

        if(nodeItem.node.fromPole == CircuitPole::Second)
        {
            //TODO: extend parent
            // if(extendParent && parent->type() == CircuitType::Open)
            // {
            //     // Extend original circuit
            //     parent->extendCircuit({nodeItem});
            // }

            // We closed the circuit
            if(parent)
                parent->promoteToType(CircuitType::Closed);

            QVector<Item> newItems = items;
            newItems.append(nodeItem);

            if(extendParent)
            {
                // Extend original circuit
                parent->extendCircuit(newItems);
            }
            else
            {
                ElectricCircuit *circuit = new ElectricCircuit();
                circuit->mItems = items;
                circuit->mItems.append(nodeItem);
                circuit->mType = CircuitType::Closed;
                circuit->setParent(parent);
                circuit->enableCircuit();
            }
        }
        return;
    }

    CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable;
    nodeSourceCable.cable.side = ~lastCable.side;
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getActiveConnections(nodeSourceCable);

    // Calculate num children
    int numChildren = 0;
    for(const auto& conn : connections)
    {
        if(!conn.cable.cable)
        {
            if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                continue; // Already has voltage

            // Circuit will pass to opposite node connector
            numChildren++;
            continue;
        }

        // Get opposite side
        CableEnd cableEnd = conn.cable.cable->getNode(~conn.cable.side);

        if(!cableEnd.node)
        {
            if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                continue; // Already has voltage

            // Circuit will pass to opposite node connector
            // And then go to next cable
            numChildren++;
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

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(parent, node, nodeContact, lastCable.pole))
            continue;
        if(containsNode(items, node, nodeContact, lastCable.pole))
            continue;

        //if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
        //    continue; // Already has voltage or current

        numChildren++;
    }

    bool usePrevItems = true;
    if(numChildren > 1 || numChildren == 0)
    {
        QVector<Item> newItems = items;
        newItems.append(nodeItem); // No toContact

        if(extendParent && parent->type() == CircuitType::Open)
        {
            // Extend original circuit
            parent->extendCircuit(newItems);
        }
        else
        {
            // Make circuit end here
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = newItems;
            circuit->mType = CircuitType::Open;
            circuit->setParent(parent);
            circuit->enableCircuit();

            // We are new parent for following circuits
            parent = circuit;
        }

        if(numChildren == 0)
            return; // No continuation

        // We cannot extend for multilple children
        // Each one will be a new circuit
        usePrevItems = false;
        extendParent = false;
    }

    for(const auto& conn : connections)
    {
        nodeItem.node.toContact = conn.nodeContact;
        nodeItem.node.toPole = conn.cable.pole;

        Item nextCable;
        nextCable.cable = conn.cable;

        if(!conn.cable.cable)
        {
            if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                continue; // Already has voltage

            QVector<Item> newItems;
            if(usePrevItems)
                newItems = items;
            newItems.append(nodeItem); // With toContact

            if(extendParent && parent->type() == CircuitType::Open)
            {
                // Extend original circuit
                parent->extendCircuit(newItems);
                break;
            }
            else
            {
                // Register an open circuit which passes through node
                ElectricCircuit *circuit = new ElectricCircuit();
                circuit->mItems = newItems;
                circuit->mType = CircuitType::Open;
                circuit->setParent(parent);
                circuit->enableCircuit();
                continue;
            }
        }

        QVector<Item> newItems;
        if(usePrevItems)
            newItems = items;
        newItems.append(nodeItem);
        newItems.append(nextCable);

        // Get opposite side
        CableEnd cableEnd = conn.cable.cable->getNode(~conn.cable.side);

        if(!cableEnd.node)
        {
            if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                continue; // Already has voltage

            if(extendParent && parent->type() == CircuitType::Open)
            {
                // Extend original circuit
                parent->extendCircuit(newItems);
            }
            else
            {
                // Register an open circuit which passes through node
                // And then go to next cable
                ElectricCircuit *circuit = new ElectricCircuit();
                circuit->mItems = newItems;
                circuit->mType = CircuitType::Open;
                circuit->setParent(parent);
                circuit->enableCircuit();
                continue;
            }
        }

        if(cableEnd.node == node && cableEnd.nodeContact == nodeContact)
        {
            continue;
        }

        if(containsNode(parent, node, nodeContact, lastCable.pole))
            continue;
        if(containsNode(items, node, nodeContact, lastCable.pole))
            continue;

        //if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
        //    continue; // Already has voltage or current

        passCircuitNode(parent,
                        cableEnd.node, cableEnd.nodeContact,
                        newItems, depth + 1, extendParent);
    }

    return;
}

void ElectricCircuit::createCircuitsFromOtherNode(AbstractCircuitNode *node)
{
    QVector<ElectricCircuit *> existingCircuits = node->getCircuits(CircuitType::Closed);
    existingCircuits.append(node->getCircuits(CircuitType::Open));

    // Search which nodes do not have any circuit.
    // We look for new circuits on these nodes
    // and ignore the others.
    QVarLengthArray<int, 4> unpoweredContacts;
    for(int contact = 0; contact < node->getContactCount(); contact++)
    {
        if(node->hasAnyExitCircuit(contact) == AnyCircuitType::None)
        {
            unpoweredContacts.append(contact);
        }
    }

    for(int circuitIdx = 0; circuitIdx < existingCircuits.size(); circuitIdx++)
    {
        ElectricCircuit *origCircuit = existingCircuits.at(circuitIdx);

        // Try to continue circuit
        if(origCircuit->mItems.size() < 3)
            continue; // Previous Node + cable + Our Node

        // This node might not be the last of this circuit.
        // So we take every passage of this circuit on this node
        // and from each we try to look for new circuits in other
        // directions than original circuit.

        for(int i = 0; i < origCircuit->mItems.size(); i++)
        {
            const Item otherItem = origCircuit->mItems.at(i);
            if(!otherItem.isNode || otherItem.node.node != node)
                continue;

            if(i == 0)
                break;

            // Let's see if this circuit can diverge and go to request contact

            CableContact lastCable = origCircuit->mItems.at(i - 1).cable;

            // Go forward
            CableItem nodeSourceCable;
            nodeSourceCable.cable = lastCable;
            nodeSourceCable.cable.side = ~lastCable.side;
            nodeSourceCable.nodeContact = otherItem.node.fromContact;
            const auto connections = node->getActiveConnections(nodeSourceCable);

            // Calculate num children
            int numChildren = 0;
            for(const auto& conn : connections)
            {
                if(!conn.cable.cable)
                {
                    if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                        continue; // Already has voltage

                    // Circuit will pass to opposite node connector
                    numChildren++;
                    continue;
                }

                // Get opposite side
                CableEnd cableEnd = conn.cable.cable->getNode(~conn.cable.side);

                if(!cableEnd.node)
                {
                    if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                        continue; // Already has voltage

                    // Circuit will pass to opposite node connector
                    // And then go to next cable
                    numChildren++;
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

                if(cableEnd.node == node && cableEnd.nodeContact == nodeSourceCable.nodeContact)
                {
                    continue;
                }

                if(containsNode(origCircuit, node, nodeSourceCable.nodeContact, lastCable.pole))
                    continue;

                numChildren++;
            }

            // Ensure slip happens only once
            bool wasSplit = false;
            bool canExtend = (numChildren == 1);

            for(const auto& conn : connections)
            {
                if(!unpoweredContacts.contains(conn.nodeContact))
                    continue;

                if(conn.nodeContact == otherItem.node.toContact)
                    continue; // We should follow a different path

                // We can start from this circuit

                if(!wasSplit && i != origCircuit->mItems.size() - 1)
                {
                    // We are not last node
                    // Split it into 2 parts
                    wasSplit = true;
                    canExtend = false;

                    // Copy items from node and after
                    QVector<Item> otherItems(origCircuit->mItems.begin() + i,
                                             origCircuit->mItems.end());

                    ElectricCircuit *continuationCircuit = new ElectricCircuit();
                    continuationCircuit->mItems = otherItems;
                    continuationCircuit->mType = origCircuit->type();
                    continuationCircuit->setParent(origCircuit);

                    const auto oldChildrenCopy = origCircuit->mChildren;
                    for(ElectricCircuit *child : oldChildrenCopy)
                    {
                        if(child == continuationCircuit)
                            continue;

                        child->setParent(continuationCircuit);
                    }

                    continuationCircuit->enableCircuit();

                    // Later we re-scan this circuit
                    existingCircuits.append(continuationCircuit);

                    // Now we disable original circuit, modify it and re-enable it
                    origCircuit->disableInternal();

                    // Remove items following node and node itself
                    origCircuit->mItems.remove(i, origCircuit->mItems.size() - i);

                    // Re-add node but without toContact
                    Item nodeItem = otherItem;
                    nodeItem.node.toContact = NodeItem::InvalidContact;
                    origCircuit->mItems.append(nodeItem);

                    // Re-enable it
                    origCircuit->enableCircuit();
                }

                Item nodeItem = otherItem;
                nodeItem.node.toContact = conn.nodeContact;
                nodeItem.node.toPole = conn.cable.pole;

                Item nextCable;
                nextCable.cable = conn.cable;

                if(!conn.cable.cable)
                {
                    if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                        continue; // Already has voltage

                    if(canExtend && origCircuit->type() == CircuitType::Open)
                    {
                        // Extend original circuit
                        origCircuit->extendCircuit({nodeItem});
                        break;
                    }
                    else
                    {
                        // Register an open circuit which passes through node
                        ElectricCircuit *circuit = new ElectricCircuit();
                        circuit->mItems.append(nodeItem);
                        circuit->mType = CircuitType::Open;
                        circuit->setParent(origCircuit);
                        circuit->enableCircuit();

                        // Circuit will pass to opposite node connector
                        continue;
                    }
                }

                // Get opposite side
                CableEnd cableEnd = conn.cable.cable->getNode(~conn.cable.side);

                if(!cableEnd.node)
                {
                    if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                        continue; // Already has voltage

                    if(canExtend && origCircuit->type() == CircuitType::Open)
                    {
                        // Extend original circuit
                        origCircuit->extendCircuit({nodeItem, nextCable});
                        break;
                    }
                    else
                    {
                        // Register an open circuit which passes through node
                        // And then go to next cable
                        ElectricCircuit *circuit = new ElectricCircuit();
                        circuit->setParent(origCircuit);
                        circuit->mItems.append(nodeItem);
                        circuit->mItems.append(nextCable);
                        circuit->mType = CircuitType::Open;
                        circuit->enableCircuit();

                        // Circuit will pass to opposite node connector
                        continue;
                    }
                }

                // At this point one of the following can happen:
                // 1- Cable is connected both sides to same node (rather impossible)
                // 2- Circuit already contain this node with same contact and polarity
                //    So it's an invalid circuit with infinite loop and must be ignored
                // 3- Circuit will pass node and go on
                // In all cases above we consider circuit as not ending here
                // So that original circuit can be freed
                // This avoid keeping around duplicate Open Circuits

                if(cableEnd.node == node && cableEnd.nodeContact == nodeSourceCable.nodeContact)
                {
                    continue;
                }

                if(containsNode(origCircuit, node, nodeSourceCable.nodeContact, lastCable.pole))
                    continue;

                QVector<Item> newItems;
                newItems.append(nodeItem);
                newItems.append(nextCable);

                passCircuitNode(origCircuit,
                                cableEnd.node, cableEnd.nodeContact,
                                newItems, 1, canExtend);

                if(wasSplit)
                    break;
            }
        }
    }
}

void ElectricCircuit::tryReachNextOpenCircuit(AbstractCircuitNode *goalNode, int nodeContact, CircuitPole pole)
{
    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = goalNode;
    nodeItem.node.fromContact = nodeContact; // Backwards
    nodeItem.node.toContact = NodeItem::InvalidContact;
    nodeItem.node.toPole = pole;

    const auto& contact = goalNode->getContacts().at(nodeContact);

    if(!contact.cable)
        return;

    Item item;
    item.cable.cable = contact.cable;
    item.cable.pole = nodeItem.node.toPole;
    item.cable.side = ~contact.cableSide; // Opposite side

    // Get opposite side node
    CableEnd cableEnd = item.cable.cable->getNode(item.cable.side);

    if(!cableEnd.node)
        return;

    QVector<Item> items;
    items.append(nodeItem);
    items.append(item);

    searchNodeWithOpenCircuits(cableEnd.node, cableEnd.nodeContact, items, 0);
}

void ElectricCircuit::defaultReachNextOpenCircuit(AbstractCircuitNode *goalNode)
{
    for(int contact = 0; contact < goalNode->getContactCount(); contact++)
    {
        if(goalNode->hasAnyCircuit(contact) == AnyCircuitType::None)
        {
            // This contact is not powered anymore
            // Let's see if it can get voltage from other nodes
            ElectricCircuit::tryReachNextOpenCircuit(goalNode,
                                                     contact,
                                                     CircuitPole::First);
            ElectricCircuit::tryReachNextOpenCircuit(goalNode,
                                                     contact,
                                                     CircuitPole::Second);
        }
    }
}

void ElectricCircuit::searchNodeWithOpenCircuits(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items, int depth)
{
    if(depth > 1000)
        return; // TODO

    const CableContact lastCable = items.constLast().cable;

    Item nodeItem;
    nodeItem.isNode = true;
    nodeItem.node.node = node;
    nodeItem.node.toContact = nodeContact; // Backwards
    nodeItem.node.toPole = lastCable.pole;

    if(node->hasCircuits(CircuitType::Open))
    {
        // This node has voltage and it's connected to us
        // Maybe we can take voltage from it, towards us
        // TODO: avoid duplicates
        extendOpenCircuits(node, nodeContact, items);
        return;
    }

    // Go backwards
    CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable; // Backwards
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getActiveConnections(nodeSourceCable, true);

    for(const auto& conn : connections)
    {
        nodeItem.node.fromContact = conn.nodeContact;
        nodeItem.node.fromPole = conn.cable.pole;

        if(!conn.cable.cable)
            continue;

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

        searchNodeWithOpenCircuits(cableEnd.node, cableEnd.nodeContact, newItems, depth + 1);
    }
}

void ElectricCircuit::extendOpenCircuits(AbstractCircuitNode *node, int nodeContact, const QVector<Item> &items)
{
    if(qobject_cast<PowerSourceNode *>(node))
    {
        // Error, different power source connected
        Q_ASSERT(false);
        return;
    }

    const CableContact lastCable = items.constLast().cable;

    const QVector<ElectricCircuit *> openCircuitsCopy = node->getCircuits(CircuitType::Open);

    for(ElectricCircuit *otherCircuit : openCircuitsCopy)
    {
        for(int i = 0; i < otherCircuit->mItems.size(); i++)
        {
            // TODO: power source

            const Item& otherItem = otherCircuit->mItems.at(i);
            if(!otherItem.isNode || otherItem.node.node != node)
                continue;

            // Let's see if this circuit can diverge and go to request contact

            // Go forward
            CableItem nodeSourceCable;
            nodeSourceCable.cable = otherCircuit->mItems.at(i - 1).cable;
            nodeSourceCable.nodeContact = otherItem.node.fromContact;
            const auto connections = node->getActiveConnections(nodeSourceCable);

            for(const auto& conn : connections)
            {
                if(conn.nodeContact != nodeContact)
                    continue;

                if(conn.cable.pole != lastCable.pole)
                    continue;

                // This circuit can reach our node!

                // Copy until cable before node
                QVector<Item> newItems(otherCircuit->mItems.begin(),
                                       otherCircuit->mItems.begin() + i);

                // Custom pass node to join with existing circuit
                Item customNodeItem = otherItem;
                customNodeItem.node.toContact = nodeContact;
                customNodeItem.node.toPole = lastCable.pole;

                newItems.append(customNodeItem);

                // Add previous searchd path from goal node (reversed)
                for(auto it = items.crbegin(); it != items.crend(); it++)
                    newItems.append(*it);

                // Register new Open circuit
                ElectricCircuit *circuit = new ElectricCircuit();
                circuit->mItems = newItems;
                circuit->mType = CircuitType::Open;
                circuit->enableCircuit();
            }
        }
    }
}

void ElectricCircuit::disableInternal()
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

    enabled = false;
}

void ElectricCircuit::disableChildren()
{
    // Disable all children
    for(ElectricCircuit *child : mChildren)
    {
        // Parent demote was already done if needed
        child->disableChildren();
        child->disableInternal();

        // We manually remove them from children list
        child->mParent = nullptr;
        delete child;
    }
    mChildren.clear();
    mClosedChildCount = 0;
}

void ElectricCircuit::extendCircuit(const QVector<Item> &items)
{
    Q_ASSERT(!mItems.isEmpty());
    Q_ASSERT(!items.isEmpty());
    Q_ASSERT(mItems.last().isNode);

    int firstToAdd = 0;
    QSet<AbstractCircuitNode *> existingNodes;

    const Item nodeItem = items.first();
    if(nodeItem.isNode && nodeItem.node.node == mItems.last().node.node)
    {
        if(enabled)
        {
            nodeItem.node.node->partialRemoveCircuit(this, {mItems.last().node});
            nodeItem.node.node->partialAddCircuit(this, {nodeItem.node});
        }

        mItems.last() = nodeItem;
        firstToAdd = 1;

        for(int i = 0; i < mItems.size(); i++)
        {
            const Item& item = mItems.at(i);
            if(item.isNode)
            {
                existingNodes.insert(item.node.node);
            }
        }
    }

    mItems.append(items.begin() + firstToAdd, items.end());

    if(enabled)
    {
        // Register on new items
        for(int i = firstToAdd; i < items.size(); i++)
        {
            const Item& item = items[i];
            if(item.isNode)
            {
                if(existingNodes.contains(item.node.node))
                    item.node.node->partialAddCircuit(this, {item.node});
                else
                    item.node.node->addCircuit(this);
            }
            else
            {
                item.cable.cable->addCircuit(this, item.cable.pole);
            }
        }
    }
}

ElectricCircuit *ElectricCircuit::parent() const
{
    return mParent;
}

void ElectricCircuit::setParent(ElectricCircuit *newParent)
{
    if(mParent == newParent)
        return;

    Q_ASSERT(newParent != this);

    if(mParent)
    {
        if(type() == CircuitType::Closed)
        {
            // TODO: Already done in disableInternal() ???
            Q_ASSERT(mParent->mClosedChildCount > 0);
            mParent->mClosedChildCount--;
        }
        mParent->mChildren.removeOne(this);
    }

    mParent = newParent;

    if(mParent)
    {
        // Open circuits can only have open circuit children
        Q_ASSERT(mParent->type() == CircuitType::Closed || type() == mParent->type());
        mParent->mChildren.append(this);
        if(type() == CircuitType::Closed)
        {
            mParent->mClosedChildCount++;
        }
    }
}

void ElectricCircuit::promoteToType(CircuitType newType)
{
    ElectricCircuit *circuit = this;
    while(circuit)
    {
        if(circuit->type() == newType)
            break;

        // Cannot demote to Open circuit if we have Closed children
        if(newType == CircuitType::Open && circuit->mClosedChildCount > 0)
            break;

        circuit->disableInternal();
        circuit->mType = newType;
        circuit->enableCircuit();

        ElectricCircuit *p = circuit->parent();
        if(p)
        {
            if(newType == CircuitType::Closed)
                p->mClosedChildCount++;
            else
            {
                Q_ASSERT(p->mClosedChildCount > 0);
                p->mClosedChildCount--;
            }
        }

        circuit = p;
    }
}
