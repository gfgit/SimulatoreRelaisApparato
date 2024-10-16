#include "simplecircuitnode.h"

#include "../closedcircuit.h"

SimpleCircuitNode::SimpleCircuitNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 8 sides (4 connectors bifilar)
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<AbstractCircuitNode::CableItem> SimpleCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact >= 8)
        return {};

    if(!isContactEnabled(source.nodeContact))
        return {};

    QVector<AbstractCircuitNode::CableItem> result;
    result.reserve(3);

    int odd = source.nodeContact % 2;
    for(int i = odd; i < 8; i += 2) // Loop contacts of same polarity
    {
        if(i == source.nodeContact)
            continue; // Skip self

        if(!isContactEnabled(i))
            continue;

        if(!mContacts.at(i).item.cable)
            continue;

        result.append(mContacts.at(i).item);
    }

    return result;
}

void SimpleCircuitNode::addCircuit(ClosedCircuit *circuit)
{
    bool updateNeeded = false;

    // A circuit may pass 2 times on same node
    // But we add it only once
    if(!mCircuits.contains(circuit))
    {
        const auto items = circuit->getNode(this);
        for(ClosedCircuit::NodeItem item : items)
        {
            int fromContact = std::floor(item.fromContact / 2.0);
            int toContact = std::floor(item.toContact / 2.0);

            if(fromContact > 0)
            {
                mCircuitCount[fromContact - 1]++;
                updateNeeded = true;
            }
            if(toContact > 0)
            {
                mCircuitCount[toContact - 1]++;
                updateNeeded = true;
            }
        }
    }

    AbstractCircuitNode::addCircuit(circuit);

    if(updateNeeded)
        emit circuitsChanged();
}

void SimpleCircuitNode::removeCircuit(ClosedCircuit *circuit)
{
    bool updateNeeded = false;

    const auto items = circuit->getNode(this);
    for(ClosedCircuit::NodeItem item : items)
    {
        int fromContact = std::floor(item.fromContact / 2.0);
        int toContact = std::floor(item.toContact / 2.0);

        if(fromContact > 0)
        {
            Q_ASSERT(mCircuitCount[fromContact - 1] > 0);
            mCircuitCount[fromContact - 1]--;
            updateNeeded = true;
        }
        if(toContact > 0)
        {
            Q_ASSERT(mCircuitCount[toContact - 1] > 0);
            mCircuitCount[toContact - 1]--;
            updateNeeded = true;
        }
    }

    AbstractCircuitNode::removeCircuit(circuit);

    if(updateNeeded)
        emit circuitsChanged();
}

void SimpleCircuitNode::setDisabledContact(int val)
{
    Q_ASSERT(val >= 0 && val < 4);
    if(mDisabledContact == val)
        return;
    mDisabledContact = val;
    emit disabledContactChanged();
}
