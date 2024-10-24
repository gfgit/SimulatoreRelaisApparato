/**
 * src/nodes/abstractcircuitnode.h
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ABSTRACTCIRCUITNODE_H
#define ABSTRACTCIRCUITNODE_H

#include <QObject>
#include <QVector>

#include "../enums/circuittypes.h"
#include "../enums/cabletypes.h"

class ElectricCircuit;

class QJsonObject;

class AbstractCircuitNode : public QObject
{
    Q_OBJECT
public:
    struct NodeContact
    {
        NodeContact(const QString& name1_ = QString(),
                    const QString& name2_ = QString())
            : name1(name1_), name2(name2_)
        {}

        QString name1;
        QString name2;

        CircuitCable *cable = nullptr;
        CableSide cableSide = CableSide::A;
        ContactType type1 = ContactType::NotConnected;
        ContactType type2 = ContactType::NotConnected;
        int closedCircuitCount = 0;
        int openCircuitCount = 0;

        inline int& count(CircuitType type)
        {
            return type == CircuitType::Closed ?
                        closedCircuitCount :
                        openCircuitCount;
        }

        inline int count(CircuitType type) const
        {
            return type == CircuitType::Closed ?
                        closedCircuitCount :
                        openCircuitCount;
        }

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
    virtual void removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items);
    virtual void partialRemoveCircuit(ElectricCircuit *circuit,
                                      const NodeOccurences &items);

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

    inline bool hasCircuits(CircuitType type = CircuitType::Closed) const
    {
        return getCircuits(type).size() > 0;
    }

    inline bool hasCircuit(int nodeContact,
                           CircuitType type = CircuitType::Closed) const
    {
        if(!hasCircuits(type))
            return false;

        Q_ASSERT(nodeContact >= 0 && nodeContact < getContactCount());
        return mContacts.at(nodeContact).count(type) > 0;
    }

    inline AnyCircuitType hasAnyCircuit(int nodeContact) const
    {
        if(hasCircuit(nodeContact, CircuitType::Closed))
            return AnyCircuitType::Closed;
        if(hasCircuit(nodeContact, CircuitType::Open))
            return AnyCircuitType::Open;
        return AnyCircuitType::None;
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

    typedef QVector<ElectricCircuit *> CircuitList;
    void disableCircuits(const CircuitList& listCopy,
                         AbstractCircuitNode *node);

    void disableCircuits(const CircuitList& listCopy,
                         AbstractCircuitNode *node,
                         const int contact);

    void truncateCircuits(const CircuitList& listCopy,
                          AbstractCircuitNode *node);

    void truncateCircuits(const CircuitList &listCopy,
                          AbstractCircuitNode *node,
                          const int contact);

    friend class ElectricCircuit;
    inline CircuitList& getCircuits(CircuitType type)
    {
        return type == CircuitType::Closed ? mClosedCircuits : mOpenCircuits;
    }

    inline const CircuitList& getCircuits(CircuitType type) const
    {
        return type == CircuitType::Closed ? mClosedCircuits : mOpenCircuits;
    }

    void unregisterOpenCircuitExit(ElectricCircuit *circuit);

private:
    CircuitList mClosedCircuits;
    CircuitList mOpenCircuits;
};

#endif // ABSTRACTCIRCUITNODE_H