#include "powersourcenode.h"

#include "../closedcircuit.h"

PowerSourceNode::PowerSourceNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 1 side
    mContacts.append(NodeContact());
}

QVector<AbstractCircuitNode::CableItem> PowerSourceNode::getActiveConnections(CableItem source, bool invertDir)
{
    // Make circuits end here
    return {};
}

QString PowerSourceNode::nodeType() const
{
    return NodeType;
}

bool PowerSourceNode::getEnabled() const
{
    return enabled;
}

void PowerSourceNode::setEnabled(bool newEnabled)
{
    if (enabled == newEnabled)
        return;
    enabled = newEnabled;
    emit enabledChanged(enabled);

    if(enabled)
    {
        ClosedCircuit::createCircuitsFromPowerNode(this);
    }
    else
    {
        const auto circuits = mCircuits;
        for(ClosedCircuit *circuit : circuits)
        {
            circuit->disableCircuit();
            delete circuit;
        }
    }
}
