#include "powersourcenode.h"

#include "../electriccircuit.h"

PowerSourceNode::PowerSourceNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));
}

QVector<CableItem> PowerSourceNode::getActiveConnections(CableItem source, bool invertDir)
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
        ElectricCircuit::createCircuitsFromPowerNode(this);
    }
    else
    {
        const auto circuits = mClosedCircuits;
        for(ElectricCircuit *circuit : circuits)
        {
            circuit->disableCircuit();
            delete circuit;
        }
    }
}
