#include "simplecircuitnode.h"

#include "../core/electriccircuit.h"

#include <QJsonObject>

SimpleCircuitNode::SimpleCircuitNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 4 sides
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
    mContacts.append(NodeContact());
}

QVector<CableItem> SimpleCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || source.nodeContact >= getContactCount())
        return {};

    if(!isContactEnabled(source.nodeContact))
        return {};

    QVector<CableItem> result;
    result.reserve(3);

    for(int i = 0; i < 4; i++) // Loop contacts of same polarity
    {
        if(i == source.nodeContact)
            continue; // Skip self

        if(!isContactEnabled(i))
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

void SimpleCircuitNode::setDisabledContact(int val)
{
    Q_ASSERT(val >= 0 && val < 4);
    if(mDisabledContact == val)
        return;
    mDisabledContact = val;
    emit disabledContactChanged();

    // Commmon cannot be disabled
    if(mDisabledContact > 0)
    {
        // Remove circuits and detach cable
        // No need to add circuits to previous disabled contact
        // Since it will not have cable attached
        if(hasCircuit(mDisabledContact) > 0)
        {
            // Disable all circuits passing on disabled contact
            const CircuitList closedCopy = getCircuits(CircuitType::Closed);
            disableCircuits(closedCopy, this, mDisabledContact);

            const CircuitList openCopy = getCircuits(CircuitType::Open);
            truncateCircuits(openCopy, this, mDisabledContact);
        }

        detachCable(mDisabledContact);
    }
}

bool SimpleCircuitNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    int val = obj.value("disabledContact").toInt();
    if(val < 0 || val >= 4)
        return false;

    setDisabledContact(val);
    return true;
}

void SimpleCircuitNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["disabledContact"] = disabledContact();
}

QString SimpleCircuitNode::nodeType() const
{
    return NodeType;
}
