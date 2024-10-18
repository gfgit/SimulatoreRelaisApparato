#include "relaispowernode.h"

#include "../abstractrelais.h"

RelaisPowerNode::RelaisPowerNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 1 side
    mContacts.append(NodeContact());
}

RelaisPowerNode::~RelaisPowerNode()
{
    if(mRelais)
        mRelais->removePowerNode(this);
}

QVector<AbstractCircuitNode::CableItem> RelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 || !mContacts.at(0).cable)
        return {};

    // Close the circuit
    // TODO: polarized relays?
    CableItem dest = source;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void RelaisPowerNode::addCircuit(ClosedCircuit *circuit)
{
    bool wasEmpty = mCircuits.isEmpty();

    AbstractCircuitNode::addCircuit(circuit);

    if(mRelais && wasEmpty && !mCircuits.isEmpty())
    {
        mRelais->powerNodeActivated(this);
    }
}

void RelaisPowerNode::removeCircuit(ClosedCircuit *circuit)
{
    bool hadCircuit = !mCircuits.isEmpty();

    AbstractCircuitNode::removeCircuit(circuit);

    if(mRelais && hadCircuit && mCircuits.isEmpty())
    {
        mRelais->powerNodeDeactivated(this);
    }
}

AbstractRelais *RelaisPowerNode::relais() const
{
    return mRelais;
}

void RelaisPowerNode::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;
    mRelais = newRelais;
    emit relayChanged();
}
