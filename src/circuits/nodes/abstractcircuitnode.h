/**
 * src/circuits/nodes/abstractcircuitnode.h
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
#include <QVarLengthArray>
#include <QVector>

#include "../../enums/circuittypes.h"
#include "../../enums/cabletypes.h"
#include "../../utils/objectproperty.h"

class ElectricCircuit;

class ModeManager;
class QJsonObject;

class AbstractCircuitNode : public QObject
{
    Q_OBJECT
public:
    typedef QVector<ElectricCircuit *> CircuitList;

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
        uint16_t closedCircuitEntranceCount[2] = {0, 0};
        uint16_t openCircuitEntranceCount[2] = {0, 0};
        uint16_t closedCircuitExitCount[2] = {0, 0};
        uint16_t openCircuitExitCount[2] = {0, 0};

        CircuitFlags mClosesFlags = CircuitFlags::None;
        CircuitFlags mOpenFlags = CircuitFlags::None;

        inline uint16_t& entranceCount(CircuitType type, CircuitPole pole)
        {
            return type == CircuitType::Closed ?
                       closedCircuitEntranceCount[int(pole)] :
                       openCircuitEntranceCount[int(pole)];
        }

        inline uint16_t entranceCount(CircuitType type, CircuitPole pole) const
        {
            return type == CircuitType::Closed ?
                       closedCircuitEntranceCount[int(pole)] :
                       openCircuitEntranceCount[int(pole)];
        }

        inline uint16_t& exitCount(CircuitType type, CircuitPole pole)
        {
            return type == CircuitType::Closed ?
                       closedCircuitExitCount[int(pole)] :
                       openCircuitExitCount[int(pole)];
        }

        inline uint16_t exitCount(CircuitType type, CircuitPole pole) const
        {
            return type == CircuitType::Closed ?
                       closedCircuitExitCount[int(pole)] :
                       openCircuitExitCount[int(pole)];
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

        inline const CircuitFlags& getFlags(CircuitType type) const
        {
            return type == CircuitType::Closed ? mClosesFlags : mOpenFlags;
        }

        inline CircuitFlags& getFlags(CircuitType type)
        {
            return type == CircuitType::Closed ? mClosesFlags : mOpenFlags;
        }
    };

    typedef QVarLengthArray<NodeContact, 4> NodeContacts;
    typedef QVarLengthArray<CableItemFlags, 4> ConnectionsRes;

    explicit AbstractCircuitNode(ModeManager *mgr, bool isLoad = false, QObject *parent = nullptr);
    ~AbstractCircuitNode();

    inline int getContactCount() const { return mContacts.size(); }

    virtual ConnectionsRes getActiveConnections(CableItem source, bool invertDir = false) = 0;

    virtual void addCircuit(ElectricCircuit *circuit);
    virtual void removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items);
    virtual void partialRemoveCircuit(ElectricCircuit *circuit,
                                      const NodeOccurences &items);

    virtual bool loadFromJSON(const QJsonObject& obj);
    virtual void saveToJSON(QJsonObject& obj) const;

    virtual void getObjectProperties(QVector<ObjectProperty> &result) const;

    // Source methods
    virtual bool isSourceNode(bool onlyCurrentState, int nodeContact = NodeItem::InvalidContact) const;
    virtual bool sourceDoNotCloseCircuits() const;
    virtual bool isSourceEnabled() const;
    virtual void setSourceEnabled(bool newEnabled);

    virtual bool tryFlipNode(bool forward);

    inline bool isElectricLoadNode() const { return isElectricLoad; }

    inline const NodeContacts &getContacts() const
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

    inline uint16_t entranceCount(int nodeContact,
                                  CircuitType type, CircuitPole pole) const
    {
        Q_ASSERT(nodeContact >= 0 && nodeContact < getContactCount());
        return mContacts.at(nodeContact).entranceCount(type, pole);
    }

    inline uint16_t exitCount(int nodeContact,
                              CircuitType type, CircuitPole pole) const
    {
        Q_ASSERT(nodeContact >= 0 && nodeContact < getContactCount());
        return mContacts.at(nodeContact).exitCount(type, pole);
    }

    inline bool hasEntranceCircuitOnPole(int nodeContact,
                                         CircuitPole pole,
                                         CircuitType type = CircuitType::Closed) const
    {
        Q_ASSERT(nodeContact >= 0 && nodeContact < getContactCount());
        return mContacts.at(nodeContact).entranceCount(type, pole) > 0;
    }

    inline bool hasExitCircuitOnPole(int nodeContact,
                                     CircuitPole pole,
                                     CircuitType type = CircuitType::Closed) const
    {
        Q_ASSERT(nodeContact >= 0 && nodeContact < getContactCount());
        return mContacts.at(nodeContact).exitCount(type, pole) > 0;
    }

    inline bool hasCircuitOnPole(int nodeContact,
                                 CircuitPole pole,
                                 CircuitType type = CircuitType::Closed) const
    {
        return hasEntranceCircuitOnPole(nodeContact, pole, type) ||
               hasExitCircuitOnPole(nodeContact, pole, type);
    }

    inline bool hasCircuit(int nodeContact,
                           CircuitType type = CircuitType::Closed) const
    {
        return hasCircuitOnPole(nodeContact, CircuitPole::First, type) ||
               hasCircuitOnPole(nodeContact, CircuitPole::Second, type);
    }

    inline bool hasCircuitsWithFlags() const
    {
        return mCircuitsWithFlags > 0;
    }

    inline CircuitFlags getCircuitFlags(int nodeContact) const
    {
        Q_ASSERT(nodeContact >= 0 && nodeContact < getContactCount());

        if(hasCircuit(nodeContact, CircuitType::Closed))
            return mContacts.at(nodeContact).getFlags(CircuitType::Closed);
        return mContacts.at(nodeContact).getFlags(CircuitType::Open);
    }

    inline AnyCircuitType hasAnyCircuitOnPole(int nodeContact,
                                              CircuitPole pole) const
    {
        if(hasCircuitOnPole(nodeContact, pole, CircuitType::Closed))
            return AnyCircuitType::Closed;
        if(hasCircuitOnPole(nodeContact, pole, CircuitType::Open))
            return AnyCircuitType::Open;
        return AnyCircuitType::None;
    }

    inline AnyCircuitType hasAnyCircuit(int nodeContact) const
    {
        if(hasCircuit(nodeContact, CircuitType::Closed))
            return AnyCircuitType::Closed;
        if(hasCircuit(nodeContact, CircuitType::Open))
            return AnyCircuitType::Open;
        return AnyCircuitType::None;
    }

    inline AnyCircuitType hasAnyEntranceCircuitOnPole(int nodeContact,
                                                      CircuitPole pole) const
    {
        if(hasEntranceCircuitOnPole(nodeContact, pole, CircuitType::Closed))
            return AnyCircuitType::Closed;
        if(hasEntranceCircuitOnPole(nodeContact, pole, CircuitType::Open))
            return AnyCircuitType::Open;
        return AnyCircuitType::None;
    }

    inline AnyCircuitType hasAnyExitCircuitOnPole(int nodeContact,
                                                  CircuitPole pole) const
    {
        if(hasExitCircuitOnPole(nodeContact, pole, CircuitType::Closed))
            return AnyCircuitType::Closed;
        if(hasExitCircuitOnPole(nodeContact, pole, CircuitType::Open))
            return AnyCircuitType::Open;
        return AnyCircuitType::None;
    }

    void attachCable(const CableItem &item);
    void detachCable(const CableItem &item);

    virtual QString nodeType() const = 0;

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    void applyNewFlags(CircuitFlags sourceFlags = CircuitFlags::None,
                       int nodeContact = NodeItem::InvalidContact);

protected:
    friend class CircuitScene;
    friend class AbstractNodeGraphItem;
    void detachCable(int contactIdx);

    bool updateCircuitFlags(int contact, CircuitType type);

    virtual void onCircuitFlagsChanged();

signals:
    void circuitsChanged();
    void shapeChanged(bool boundingRectChange = false, bool cableChange = false);

protected:
    NodeContacts mContacts;
    int mCircuitsWithFlags = 0;

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
    ModeManager *mModeMgr;

    CircuitList mClosedCircuits;
    CircuitList mOpenCircuits;
    const bool isElectricLoad;
};

#endif // ABSTRACTCIRCUITNODE_H
