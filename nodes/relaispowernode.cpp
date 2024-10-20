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
    setRelais(nullptr);
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

    if(mRelais)
        mRelais->removePowerNode(this);
    mRelais = newRelais;
    if(mRelais)
        mRelais->addPowerNode(this);

    emit relayChanged(mRelais);
}

RelaisModel *RelaisPowerNode::relaisModel() const
{
    return mRelaisModel;
}

void RelaisPowerNode::setRelaisModel(RelaisModel *newRelaisModel)
{
    mRelaisModel = newRelaisModel;
}
