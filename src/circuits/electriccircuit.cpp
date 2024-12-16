/**
 * src/circuits/electriccircuit.cpp
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

#include "electriccircuit.h"

#include "nodes/circuitcable.h"
#include "nodes/powersourcenode.h"

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
    allCircuitsCount--;
}

void ElectricCircuit::enableCircuit()
{
    Q_ASSERT(!enabled);
    Q_ASSERT(!mItems.isEmpty());
    Q_ASSERT(getSource());

    // Check for duplicate circuits
    // TODO: this should not happen
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

ElectricCircuit::PassNodeResult ElectricCircuit::passCircuitNode(AbstractCircuitNode *node, int nodeContact,
                                                                 const QVector<Item> &items, int depth,
                                                                 PassMode mode)
{
    // Returns true if circuit goes to next node
    if(depth > 1000)
        return {}; // TODO

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
            return {0, 1};
        }
        return {};
    }

    if(qobject_cast<PowerSourceNode *>(node))
    {
        // Error, different power source connected
        return {};
    }

    CableItem nodeSourceCable;
    nodeSourceCable.cable = lastCable;
    nodeSourceCable.cable.side = ~lastCable.side;
    nodeSourceCable.nodeContact = nodeContact;
    const auto connections = node->getActiveConnections(nodeSourceCable);

    bool circuitEndsHere = true;

    PassNodeResult result;

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

            if(mode.testFlag(PassModes::ReverseVoltagePassed))
                continue;

            // Register an open circuit which passes through node
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = items;
            circuit->mItems.append(nodeItem);
            circuit->mType = CircuitType::Open;
            circuit->enableCircuit();

            // Circuit will pass to opposite node connector
            circuitEndsHere = false;
            result.openCircuits++;
            continue;
        }

        // Get opposite side
        CableEnd cableEnd = conn.cable.cable->getNode(~conn.cable.side);

        if(!cableEnd.node)
        {
            if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                continue; // Already has voltage

            if(mode.testFlag(PassModes::ReverseVoltagePassed))
                continue;

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
            result.openCircuits++;
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

        PassMode newMode = mode;

        if(cableEnd.node->isElectricLoad)
        {
            if(mode.testFlag(PassModes::SkipLoads))
                continue; // Skip loads

            newMode.setFlag(PassModes::LoadPassed, true);
        }

        if(cableEnd.node->hasAnyEntranceCircuit(cableEnd.nodeContact) != AnyCircuitType::None)
        {
            // We are going in, where another circuit goes out
            // So we are basically going towards higher potential
            // This is not possible, current will not flow reverse to voltage
            // So we set a flag which will prevent open circuit to be registered

            // TODO: we should not register closed circuits either
            // TODO: we should detect shortcircuit and remove power from all
            // affected circuits

            newMode.setFlag(PassModes::ReverseVoltagePassed, true);
        }

        PassNodeResult nextResult;

        if(newMode.testFlag(PassModes::LoadPassed))
        {
            // Try first to skip other loads and go directly to source
            PassMode skipLoads = newMode;
            skipLoads.setFlag(PassModes::SkipLoads, true);

            nextResult = passCircuitNode(cableEnd.node, cableEnd.nodeContact,
                                         newItems, depth + 1,
                                         skipLoads);
        }

        if(nextResult.closedCircuits == 0 && !newMode.testFlag(PassModes::SkipLoads))
        {
            // Try again allowing other loads on circuit
            newMode.setFlag(PassModes::SkipLoads, false);

            nextResult += passCircuitNode(cableEnd.node, cableEnd.nodeContact,
                                          newItems, depth + 1,
                                          newMode);
        }

        result += nextResult;
    }

    if(circuitEndsHere)
    {
        if(depth > 0 && !mode.testFlag(PassModes::ReverseVoltagePassed))
        {
            // Register an open circuit which passes HALF node
            ElectricCircuit *circuit = new ElectricCircuit();
            circuit->mItems = items;
            circuit->mItems.append(nodeItem);
            circuit->mType = CircuitType::Open;
            circuit->enableCircuit();
        }

        // Do not count this as result
        // It will replace parent circuit
        return {};
    }

    return result;
}

void ElectricCircuit::createCircuitsFromOtherNode(AbstractCircuitNode *node)
{
    const QVector<ElectricCircuit *> openCircuitsCopy = node->getCircuits(CircuitType::Open);

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

    for(ElectricCircuit *origCircuit : openCircuitsCopy)
    {
        // Try to continue circuit
        Q_ASSERT(origCircuit->mItems.size() >= 3); // PowerSource + cable + Our Node
        Q_ASSERT(origCircuit->mItems.last().isNode);

        // This node might not be the last of this circuit.
        // So we take every passage of this circuit on this node
        // and from each we try to look for new circuits in other
        // directions than original circuit.

        bool removeOriginalCircuit = false;

        bool loadPassed = false;

        for(int i = 0; i < origCircuit->mItems.size(); i++)
        {
            const Item& otherItem = origCircuit->mItems.at(i);
            if(!otherItem.isNode)
                continue;

            if(otherItem.node.node->isElectricLoad)
                loadPassed = true;

            if(otherItem.node.node != node)
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

            bool circuitEndsHere = true;

            for(const auto& conn : connections)
            {
                if(!unpoweredContacts.contains(conn.nodeContact))
                    continue;

                if(conn.nodeContact == otherItem.node.toContact)
                    continue; // We should follow a different path

                // Copy items until cable before node
                QVector<Item> items(origCircuit->mItems.begin(),
                                    origCircuit->mItems.begin() + i);

                Item nodeItem = otherItem;
                nodeItem.node.toContact = conn.nodeContact;
                nodeItem.node.toPole = conn.cable.pole;

                Item nextCable;
                nextCable.cable = conn.cable;

                if(!conn.cable.cable)
                {
                    if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                        continue; // Already has voltage

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
                    if(node->hasAnyExitCircuit(conn.nodeContact) != AnyCircuitType::None)
                        continue; // Already has voltage

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

                if(cableEnd.node == node && cableEnd.nodeContact == nodeSourceCable.nodeContact)
                {
                    continue;
                }

                if(containsNode(items, node, nodeSourceCable.nodeContact, lastCable.pole))
                    continue;

                QVector<Item> newItems = items;
                newItems.append(nodeItem);
                newItems.append(nextCable);

                PassMode mode = PassModes::None;
                if(loadPassed || cableEnd.node->isElectricLoad)
                    mode.setFlag(PassModes::LoadPassed, true);

                passCircuitNode(cableEnd.node, cableEnd.nodeContact,
                                newItems, 1,
                                mode);
            }

            if(i == origCircuit->mItems.size() - 1 && !circuitEndsHere)
            {
                // We are last node of original circuit
                // And circuits go on.
                // So remove original circuit
                removeOriginalCircuit = true;
            }
        }

        if(removeOriginalCircuit)
        {
            // Circuit went ahead, delete old open circuit
            QVector<ElectricCircuit *> dummy;
            origCircuit->terminateHere(origCircuit->getSource(), dummy);
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
