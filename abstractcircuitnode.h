#ifndef ABSTRACTCIRCUITNODE_H
#define ABSTRACTCIRCUITNODE_H

#include <QObject>
#include <QVector>

#include "enums/circuittypes.h"
#include "enums/cabletypes.h"

class ElectricCircuit;

class QJsonObject;

class AbstractCircuitNode : public QObject
{
    Q_OBJECT
public:
    struct NodeContact
    {
        QString name1;
        QString name2;

        CircuitCable *cable = nullptr;
        CableSide cableSide = CableSide::A;
        ContactType type1 = ContactType::NotConnected;
        ContactType type2 = ContactType::NotConnected;
        int circuitsCount = 0;

        inline ContactType getType(CircuitPole pole) const
        {
            if(!cable)
                return ContactType::NotConnected;
            return pole == CircuitPole::First ? type1 : type2;
        }

        inline void setType(CircuitPole pole, ContactType t)
        {
            if(pole == CircuitPole::First)
                type1 = t;
            else
                type2 = t;
        }
    };

    explicit AbstractCircuitNode(QObject *parent = nullptr);
    ~AbstractCircuitNode();

    inline int getContactCount() const { return mContacts.size(); }

    virtual QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) = 0;

    virtual void addCircuit(ElectricCircuit *circuit);
    virtual void removeCircuit(ElectricCircuit *circuit);

    virtual bool loadFromJSON(const QJsonObject& obj);
    virtual void saveToJSON(QJsonObject& obj) const;

    inline const QVector<NodeContact> &getContacts() const
    {
        return mContacts;
    }

    inline ContactType getContactType(int idx, CircuitPole pole) const
    {
        if(idx < 0 || idx >= mContacts.size())
            return ContactType::NotConnected;
        return mContacts.at(idx).getType(pole);
    }

    inline bool hasCircuits() const { return mCircuits.size(); }

    inline bool hasCircuit(int nodeContact) const
    {
        if(!hasCircuits())
            return false;

        Q_ASSERT(nodeContact >= 0 && nodeContact < getContactCount());
        return mContacts.at(nodeContact).circuitsCount > 0;
    }

    void attachCable(const CableItem &item);
    void detachCable(const CableItem &item);

    virtual QString nodeType() const = 0;

protected:
    friend class CircuitScene;
    friend class AbstractNodeGraphItem;
    void detachCable(int contactIdx);

signals:
    void circuitsChanged();

protected:
    QVector<NodeContact> mContacts;

    QVector<ClosedCircuit *> mCircuits;
};

#endif // ABSTRACTCIRCUITNODE_H
