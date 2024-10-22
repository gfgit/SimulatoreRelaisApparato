#include "relaispowernode.h"

#include "../abstractrelais.h"
#include "../relaismodel.h"

#include <QJsonObject>

RelaisPowerNode::RelaisPowerNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 1 side
    mContacts.append(NodeContact("41", "42"));
}

RelaisPowerNode::~RelaisPowerNode()
{
    setRelais(nullptr);
}

QVector<CableItem> RelaisPowerNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0)
        return {};

    // Close the circuit
    // TODO: polarized relays?
    CableItem dest;
    dest.cable.cable = mContacts.at(0).cable;
    dest.cable.side = mContacts.at(0).cableSide;
    dest.nodeContact = 0;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void RelaisPowerNode::addCircuit(ElectricCircuit *circuit)
{
    const CircuitList &closedCircuits = getCircuits(CircuitType::Closed);
    bool wasEmpty = closedCircuits.isEmpty();

    AbstractCircuitNode::addCircuit(circuit);

    if(mRelais && wasEmpty && !closedCircuits.isEmpty())
    {
        mRelais->powerNodeActivated(this);
    }
}

void RelaisPowerNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const CircuitList &closedCircuits = getCircuits(CircuitType::Closed);
    bool hadCircuit = !closedCircuits.isEmpty();

    AbstractCircuitNode::removeCircuit(circuit, items);

    if(mRelais && hadCircuit && closedCircuits.isEmpty())
    {
        mRelais->powerNodeDeactivated(this);
    }
}

bool RelaisPowerNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString relaisName = obj.value("relais").toString();

    setRelais(relaisModel()->getRelay(relaisName));

    return true;
}

void RelaisPowerNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();
}

QString RelaisPowerNode::nodeType() const
{
    return NodeType;
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
