#include "relaiscontactnode.h"

#include "../electriccircuit.h"
#include "../abstractrelais.h"

#include "../relaismodel.h"

#include <QJsonObject>

RelaisContactNode::RelaisContactNode(QObject *parent)
    : AbstractCircuitNode{parent}
{
    // 3 sides
    mContacts.append(NodeContact()); // Common
    mContacts.append(NodeContact()); // Up
    mContacts.append(NodeContact()); // Down
}

RelaisContactNode::~RelaisContactNode()
{
    setRelais(nullptr);
}

QVector<CableItem> RelaisContactNode::getActiveConnections(CableItem source, bool invertDir)
{
    if((source.nodeContact < 0) || (source.nodeContact >= getContactCount()))
        return {};

    int otherContactIdx = -1;

    const NodeContact& sourceContact = mContacts.at(source.nodeContact);
    if(sourceContact.getType(source.cable.pole) == ContactType::Passthrough &&
            (source.nodeContact == 0 || source.nodeContact == 2))
    {
        // Pass to other contact straight
        otherContactIdx = source.nodeContact == 0 ? 2 : 0;
    }
    else if(mState != State::Middle)
    {
        bool isDown = mState == State::Down;
        if(swapContactState())
            isDown = mState == State::Up;

        switch (source.nodeContact)
        {
        case 0:
            otherContactIdx = isDown ? 2 : 1;
            break;
        case 1:
            if(!isDown)
                otherContactIdx = 0;
            break;
        case 2:
            if(isDown)
                otherContactIdx = 0;
            break;
        default:
            break;
        }
    }

    if(otherContactIdx != -1)
    {
        const NodeContact& otherContact = mContacts.at(otherContactIdx);
        CableItem other;
        other.cable.cable = otherContact.cable;
        other.cable.side = otherContact.cableSide;
        other.cable.pole = source.cable.pole;
        other.nodeContact = otherContactIdx;

        if(other.cable.cable)
            return {other};
    }

    return {};
}

bool RelaisContactNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString relaisName = obj.value("relais").toString();

    setRelais(relaisModel()->getRelay(relaisName));

    setFlipContact(obj.value("flip").toBool());
    setSwapContactState(obj.value("swap_state").toBool());
    setHasCentralConnector(obj.value("central_connector").toBool(true));

    return true;
}

void RelaisContactNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);

    obj["relais"] = mRelais ? mRelais->name() : QString();
    obj["flip"] = flipContact();
    obj["swap_state"] = swapContactState();
    obj["central_connector"] = hasCentralConnector();
}

QString RelaisContactNode::nodeType() const
{
    return NodeType;
}

AbstractRelais *RelaisContactNode::relais() const
{
    return mRelais;
}

void RelaisContactNode::setRelais(AbstractRelais *newRelais)
{
    if(mRelais == newRelais)
        return;

    if(mRelais)
    {
        disconnect(mRelais, &AbstractRelais::stateChanged,
                   this, &RelaisContactNode::onRelaisStateChanged);

        mRelais->removeContactNode(this);
    }

    mRelais = newRelais;

    if(mRelais)
    {
        connect(mRelais, &AbstractRelais::stateChanged,
                this, &RelaisContactNode::onRelaisStateChanged);

        mRelais->addContactNode(this);
    }

    emit relayChanged(mRelais);
    onRelaisStateChanged();
}

RelaisModel *RelaisContactNode::relaisModel() const
{
    return mRelaisModel;
}

void RelaisContactNode::setRelaisModel(RelaisModel *newRelaisModel)
{
    mRelaisModel = newRelaisModel;
}

bool RelaisContactNode::swapContactState() const
{
    return mSwapContactState;
}

void RelaisContactNode::setSwapContactState(bool newSwapContactState)
{
    if(mSwapContactState == newSwapContactState)
        return;
    mSwapContactState = newSwapContactState;
    emit shapeChanged();
}

bool RelaisContactNode::flipContact() const
{
    return mFlipContact;
}

void RelaisContactNode::setFlipContact(bool newFlipContact)
{
    if(mFlipContact == newFlipContact)
        return;
    mFlipContact = newFlipContact;
    emit shapeChanged();
}

bool RelaisContactNode::hasCentralConnector() const
{
    return mHasCentralConnector;
}

void RelaisContactNode::setHasCentralConnector(bool newHasCentralConnector)
{
    if(mHasCentralConnector == newHasCentralConnector)
        return;
    mHasCentralConnector = newHasCentralConnector;
    emit shapeChanged();

    if(!mHasCentralConnector)
    {
        // Remove circuits and detach cable
        // No need to re-add circuits later
        // Since it will not have cable attached
        if(hasCircuit(1) > 0)
        {
            // Disable all circuits passing on disabled contact
            const auto circuits = mClosedCircuits;
            for(ElectricCircuit *circuit : circuits)
            {
                const auto items = circuit->getNode(this);
                for(ElectricCircuit::NodeItem item : items)
                {
                    if(item.fromContact == 1 || item.toContact == 1)
                    {
                        circuit->disableCircuit();
                        delete circuit;
                    }
                }
            }
        }

        detachCable(1);
    }
}

RelaisContactNode::State RelaisContactNode::state() const
{
    return mState;
}

void RelaisContactNode::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;
    emit stateChanged();

    // Delete old circuits
    const auto circuits = mClosedCircuits;
    for(ElectricCircuit *circuit : circuits)
    {
        circuit->disableCircuit();
        delete circuit;
    }

    if(mState != State::Middle)
    {
        // Scan for new circuits
        QVector<NodeContact> contactsToScan;
        contactsToScan.append(mContacts[0]);
        ElectricCircuit::createCircuitsFromOtherNode(this, contactsToScan);
    }
}

void RelaisContactNode::onRelaisStateChanged()
{
    State s = State::Middle;
    if(mRelais)
    {
        switch (mRelais->state())
        {
        case AbstractRelais::State::Down:
            s = State::Down;
            break;
        case AbstractRelais::State::Up:
            s = State::Up;
            break;
        default:
            break;
        }
    }

    setState(s);
}
