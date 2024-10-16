#include "relaispowernode.h"

#include "../abstractrelais.h"

RelaisPowerNode::RelaisPowerNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 2 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

RelaisPowerNode::~RelaisPowerNode()
{
    if(mRelais)
        mRelais->removePowerNode(this);
}

QVector<AbstractCircuitNode::CableItem> RelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 && source.nodeContact != 1)
        return {};

    // Close the circuit
    if(source.nodeContact == 0 && mContacts.at(1).item.cable)
        return {mContacts.at(1).item};

    if(source.nodeContact == 1 && mContacts.at(0).item.cable)
        return {mContacts.at(0).item};

    return {};
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
