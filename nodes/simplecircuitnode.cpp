#include "simplecircuitnode.h"

#include "../closedcircuit.h"

SimpleCircuitNode::SimpleCircuitNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 4 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<AbstractCircuitNode::CableItem> SimpleCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact >= getContactCount())
        return {};

    if(!isContactEnabled(source.nodeContact))
        return {};

    QVector<AbstractCircuitNode::CableItem> result;
    result.reserve(3);

    for(int i = 0; i < 4; i++) // Loop contacts of same polarity
    {
        if(i == source.nodeContact)
            continue; // Skip self

        if(!isContactEnabled(i))
            continue;

        if(!mContacts.at(i).cable)
            continue;

        CableItem dest;
        dest.cable.cable = mContacts.at(i).cable;
        dest.cable.side = mContacts.at(i).cableSide;
        dest.nodeContact = i;
        dest.cable.pole = source.cable.pole;

        result.append(dest);
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
            if(item.fromContact > 0)
            {
                mCircuitCount[item.fromContact - 1]++;
                updateNeeded = true;
            }
            if(item.toContact > 0)
            {
                mCircuitCount[item.toContact - 1]++;
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
        if(item.fromContact > 0)
        {
            Q_ASSERT(mCircuitCount[item.fromContact - 1] > 0);
            mCircuitCount[item.fromContact - 1]--;
            updateNeeded = true;
        }
        if(item.toContact > 0)
        {
            Q_ASSERT(mCircuitCount[item.toContact - 1] > 0);
            mCircuitCount[item.toContact - 1]--;
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

    if(mDisabledContact > 0)
    {
        if(mCircuitCount[mDisabledContact - 1] > 0)
        {
            // Disable all circuits passing on disabled contact
            const auto circuits = mCircuits;
            for(ClosedCircuit *circuit : circuits)
            {
                const auto items = circuit->getNode(this);
                for(ClosedCircuit::NodeItem item : items)
                {
                    if(item.fromContact == mDisabledContact || item.toContact == mDisabledContact)
                    {
                        circuit->disableCircuit();
                        delete circuit;
                    }
                }
            }
        }

        detachCable(mDisabledContact);
    }
}
