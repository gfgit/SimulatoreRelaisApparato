/**
 * src/graph/circuitscene.cpp
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

#include "circuitscene.h"

#include "abstractnodegraphitem.h"

#include "powersourcegraphitem.h"
#include "../nodes/powersourcenode.h"
#include "../graph/cablegraphitem.h"
#include "../nodes/circuitcable.h"

#include <QGraphicsPathItem>
#include <QPen>

#include <unordered_set>

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

#include <QJsonObject>
#include <QJsonArray>

#include "../nodes/edit/nodeeditfactory.h"

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene{parent}
{

}

CircuitScene::~CircuitScene()
{
    removeAllItems();
}

CircuitScene::Mode CircuitScene::mode() const
{
    return mMode;
}

void CircuitScene::setMode(Mode newMode)
{
    if (mMode == newMode)
        return;

    Mode oldMode = mMode;
    mMode = newMode;
    emit modeChanged(mMode);

    if(oldMode == Mode::Editing)
    {
        stopUnfinishedOperations();
        calculateConnections();
    }

    const bool powerSourceEnabled = mMode == Mode::Simulation;
    for(PowerSourceGraphItem *powerSource : std::as_const(mPowerSources))
    {
        powerSource->node()->setEnabled(powerSourceEnabled);
    }
}

void CircuitScene::addNode(AbstractNodeGraphItem *item)
{
    stopUnfinishedOperations();

    if(!isLocationFree(item->location()))
    {
        // Assign a new location to node
        item->setLocation(getNewFreeLocation());
    }

    mItemMap.insert({item->location(), item});

    // Add item after having inserted it in the map
    addItem(item);

    PowerSourceGraphItem *powerSource = qobject_cast<PowerSourceGraphItem *>(item);
    if(powerSource)
    {
        powerSource->node()->setEnabled(mMode == Mode::Simulation);
        mPowerSources.append(powerSource);
    }
}

void CircuitScene::removeNode(AbstractNodeGraphItem *item)
{
    if(item == itemBeingMoved())
        endMovingItem();

    Q_ASSERT(getNodeAt(item->location()) == item);

    removeItem(item);
    mItemMap.erase(item->location());

    PowerSourceGraphItem *powerSource = qobject_cast<PowerSourceGraphItem *>(item);
    if(powerSource)
    {
        powerSource->node()->setEnabled(false);
        mPowerSources.removeOne(powerSource);
    }

    auto *node = item->getAbstractNode();
    delete item;
    delete node;
}

void CircuitScene::addCable(CableGraphItem *item)
{
    stopUnfinishedOperations();

    Q_ASSERT(item->cable());

    if(!cablePathIsValid(item->cablePath(), nullptr))
        return; // TODO: error

    addItem(item);
    mCables.insert({item->cable(), item});

    // Add cable tiles
    addCableTiles(item);
}

void CircuitScene::removeCable(CircuitCable *cable)
{
    auto it = mCables.find(cable);
    if(it != mCables.end())
    {
        // Delete graph item
        CableGraphItem *item = it->second;
        mCables.erase(it);

        removeCableTiles(item);

        if(item == mEditingCable)
            endEditCable(false);

        delete item;
    }

    delete cable;
}

CableGraphItem *CircuitScene::graphForCable(CircuitCable *cable) const
{
    auto it = mCables.find(cable);
    if(it == mCables.end())
        return nullptr;
    return it->second;
}

TileLocation CircuitScene::getNewFreeLocation()
{
    TileLocation location{0, 0};
    while(!isLocationFree(location))
        location.x++;
    return location;
}

bool CircuitScene::isLocationFree(TileLocation l) const
{
    if(getItemAt(l))
        return false;

    auto pair = getCablesAt(l);
    if(pair.first || pair.second)
        return false;

    return true;
}

AbstractNodeGraphItem *CircuitScene::getNodeAt(TileLocation l) const
{
    auto it = mItemMap.find(l);
    if(it == mItemMap.cend())
        return nullptr;
    return it->second;
}

CircuitScene::TileCablePair CircuitScene::getCablesAt(TileLocation l) const
{
    auto it = mCableTiles.find(l);
    if(it == mCableTiles.cend())
        return {nullptr, nullptr};
    return it->second;
}

bool CircuitScene::cablePathIsValid(const CableGraphPath &cablePath, CableGraphItem *item) const
{
    // Check all tiles are free
    if(cablePath.isZeroLength())
        return true;

    std::unordered_set<TileLocation, TileLocationHash> repeatedTiles;

    for(int i = 0; i < cablePath.getTilesCount(); i++)
    {
        TileLocation tile = cablePath.at(i);

        if(getItemAt(tile))
        {
            return false;
        }

        TileCablePair pair = getCablesAt(tile);
        if(item)
        {
            // Do not consider ourselves
            if(pair.first == item)
                pair.first = nullptr;
            if(pair.second == item)
                pair.second = nullptr;
        }

        if(pair.first && pair.second)
        {
            // There are already 2 cables on this tile
            return false;
        }

        if(repeatedTiles.find(tile) != repeatedTiles.cend())
        {
            // Cable passes 2 times on same tile
            // Then this tile cannot have other cables
            if(pair.first || pair.second)
                return false;
        }
        repeatedTiles.insert(tile);

        CableGraphItem *otherCable = pair.first ? pair.first : pair.second;
        if(otherCable)
        {
            const auto& otherPath = otherCable->cablePath();
            int otherIdx = otherPath.tiles().indexOf(tile);
            Q_ASSERT(otherIdx >= 0);

            const bool isLastTile = i == cablePath.getTilesCount() - 1;
            const bool canCheckExit = !isLastTile || cablePath.isComplete();

            // Check if first passage was straigh
            const auto enterDir1 = cablePath.getEnterDirection(i);
            const auto exitDir1 = cablePath.getExitDirection(i);
            if(canCheckExit && enterDir1 != ~exitDir1)
            {
                // Directions are not opposite, it bends
                return false;
            }

            const auto enterDir2 = otherPath.getEnterDirection(otherIdx);
            const auto exitDir2 = otherPath.getExitDirection(otherIdx);
            if(enterDir2 != ~exitDir2)
            {
                // Directions are not opposite, it bends
                return false;
            }

            if(enterDir1 == enterDir2 || enterDir1 == exitDir2)
                return false; // Both passages have same direction
        }
    }

    return true;
}

bool CircuitScene::updateItemLocation(TileLocation newLocation, AbstractNodeGraphItem *item)
{
    // Only current moving item can pass (temporarily)
    // on occupied locations to allow jump them
    bool allowed = isLocationFree(newLocation);
    if(!allowed)
    {
        if(mItemBeingMoved != item)
            return false;

        // Fake allowing invalid move for currently moving item.
        // We do not update the map so it will be reverted
        // to it's last valid position on move end.
        return true;
    }

    // For untracked items, use last location
    TileLocation oldLocation = item->location();

    if(mItemBeingMoved == item)
    {
        // For moved items use last valid location
        // to remove old map entry
        oldLocation = mLastMovedItemValidLocation;

        // Store location to revert future invalid moves
        mLastMovedItemValidLocation = newLocation;
    }

    Q_ASSERT(getItemAt(oldLocation) == item);

    // Update location in map
    mItemMap.erase(oldLocation);
    mItemMap.insert({newLocation, item});

    return true;
}

void CircuitScene::calculateConnections()
{
    QVector<CircuitCable *> verifiedCables;

    auto cableCopy = mCables;
    for(const auto& it : cableCopy)
    {
        CableGraphItem *item = it.second;
        if(checkCable(item))
        {
            verifiedCables.append(item->cable());
        }
        else
        {
            if(item->cableZeroLength())
            {
                // Unconnected zero length cable, delete it
                removeCable(item->cable());
            }
        }
    }

    for(const auto& it : mItemMap)
    {
        AbstractNodeGraphItem *item = it.second;
        checkItem(item, verifiedCables);
    }

    // Delete unconnected zero length cables
    // Copy to avoid invalidating iterators while looping
    cableCopy = mCables;
    for(auto it = cableCopy.begin(); it != cableCopy.end(); it++)
    {
        CableGraphItem *item = it->second;
        if(item->cableZeroLength() && !verifiedCables.contains(item->cable()))
        {
            removeCable(item->cable());
        }
    }
}

void CircuitScene::connectItems(AbstractCircuitNode *node1, AbstractCircuitNode *node2, const Connector &c1, const Connector &c2, QVector<CircuitCable *> &verifiedCables)
{
    const auto& contacts1 = node1->getContacts();
    const auto& contacts2 = node2->getContacts();

    CircuitCable *cableA = contacts1.at(c1.nodeContact).cable;

    if(!cableA || !verifiedCables.contains(cableA))
    {
        CircuitCable *cableB = contacts2.at(c2.nodeContact).cable;
        if(cableB != cableA || !cableB)
        {
            if(cableA)
            {
                node1->detachCable(c1.nodeContact);

                auto item = graphForCable(cableA);
                if(!item || item->cableZeroLength())
                {
                    removeCable(cableA);
                    cableA = nullptr;
                }
            }

            if(cableB)
            {
                node2->detachCable(c2.nodeContact);

                auto item = graphForCable(cableB);
                if(!item || item->cableZeroLength())
                {
                    removeCable(cableB);
                    cableB = nullptr;
                }
            }

            CircuitCable *newCable = new CircuitCable(this);

            // First we set graph path
            CableGraphItem *item = new CableGraphItem(newCable);
            item->setPos(0, 0);
            item->setCablePath(CableGraphPath::createZeroLength(c1.location,
                                                                c2.location));

            // Then we create cable connection
            CableItem cableItem;
            cableItem.cable.cable = newCable;
            cableItem.cable.side = CableSide::A;
            cableItem.nodeContact = c1.nodeContact;
            cableItem.cable.pole = CircuitPole::First;
            node1->attachCable(cableItem);

            cableItem.cable.pole = CircuitPole::Second;
            node1->attachCable(cableItem);

            cableItem.cable.side = CableSide::B;
            cableItem.nodeContact = c2.nodeContact;
            cableItem.cable.pole = CircuitPole::First;
            node2->attachCable(cableItem);

            cableItem.cable.pole = CircuitPole::Second;
            node2->attachCable(cableItem);

            cableA = newCable;
            verifiedCables.append(cableA);

            addCable(item);
        }
    }
}

bool CircuitScene::checkCable(CableGraphItem *item)
{
    if(item->sideA() == TileLocation::invalid || item->sideB() == TileLocation::invalid)
        return false;

    TileLocation nodeLocA = item->sideA();
    TileLocation nodeLocB = item->sideB();

    AbstractNodeGraphItem *itemA = getItemAt(nodeLocA);
    AbstractNodeGraphItem *itemB = getItemAt(nodeLocB);

    AbstractCircuitNode *nodeA = itemA ? itemA->getAbstractNode() : nullptr;
    AbstractCircuitNode *nodeB = itemB ? itemB->getAbstractNode() : nullptr;

    CircuitCable *cable = item->cable();

    std::vector<Connector> connectorsA;
    std::vector<Connector> connectorsB;

    std::vector<Connector>::const_iterator connA = connectorsA.cend();
    std::vector<Connector>::const_iterator connB = connectorsB.cend();

    if(itemA)
    {
        itemA->getConnectors(connectorsA);

        connA = std::find_if(connectorsA.cbegin(), connectorsA.cend(),
                             [item](const Connector& c)
        {
            return c.direction == ~item->directionA();
        });
    }

    if(itemB)
    {
        itemB->getConnectors(connectorsB);

        connB = std::find_if(connectorsB.cbegin(), connectorsB.cend(),
                             [item](const Connector& c)
        {
            return c.direction == ~item->directionB();
        });
    }

    if(!nodeA || connA == connectorsA.cend())
    {
        // Detach side A
        CableEnd end = cable->getNode(CableSide::A);
        if(end.node)
        {
            end.node->detachCable(end.nodeContact);
        }

        nodeA = nullptr;
    }

    if(!nodeB || connB == connectorsB.cend())
    {
        // Detach side B
        CableEnd end = cable->getNode(CableSide::B);
        if(end.node)
        {
            end.node->detachCable(end.nodeContact);
        }

        nodeB = nullptr;
    }

    bool sideConnectedA = false;
    if(nodeA)
    {
        const auto& contactA = nodeA->getContacts().at(connA->nodeContact);

        if(contactA.cable == item->cable() && contactA.cableSide == CableSide::B)
        {
            // We have a swapped cable, detach and let it rewire later
            CableEnd end = cable->getNode(CableSide::A);
            if(end.node)
            {
                end.node->detachCable(end.nodeContact);
            }

            end = cable->getNode(CableSide::B);
            if(end.node)
            {
                end.node->detachCable(end.nodeContact);
            }
        }

        if(contactA.cable == item->cable())
        {
            sideConnectedA = true;
        }
        else if(!contactA.cable && !item->cable()->getNode(CableSide::A).node)
        {
            // Make the connection
            CableItem cableItem;
            cableItem.cable.cable = item->cable();
            cableItem.cable.side = CableSide::A;
            cableItem.nodeContact = connA->nodeContact;
            cableItem.cable.pole = CircuitPole::First;
            nodeA->attachCable(cableItem);

            cableItem.cable.pole = CircuitPole::Second;
            nodeA->attachCable(cableItem);

            sideConnectedA = true;
        }
    }

    bool sideConnectedB = false;
    if(nodeB)
    {
        const auto& contactB = nodeB->getContacts().at(connB->nodeContact);

        if(contactB.cable == item->cable())
        {
            sideConnectedB = true;
        }
        else if(!contactB.cable && !item->cable()->getNode(CableSide::B).node)
        {
            // Make the connection
            CableItem cableItem;
            cableItem.cable.cable = item->cable();
            cableItem.cable.side = CableSide::B;
            cableItem.nodeContact = connB->nodeContact;
            cableItem.cable.pole = CircuitPole::First;
            nodeB->attachCable(cableItem);

            cableItem.cable.pole = CircuitPole::Second;
            nodeB->attachCable(cableItem);

            sideConnectedB = true;
        }
    }

    return sideConnectedA && sideConnectedB;
}

void CircuitScene::checkItem(AbstractNodeGraphItem *item, QVector<CircuitCable *> &verifiedCables)
{
    const TileLocation location = item->location();

    AbstractCircuitNode *node1 = item->getAbstractNode();
    if(!node1)
        return;

    std::vector<Connector> connectors;
    item->getConnectors(connectors);

    for(const Connector& c1 : connectors)
    {
        const TileLocation otherLocation = location + c1.direction;
        AbstractNodeGraphItem *other = getItemAt(otherLocation);
        AbstractCircuitNode *node2 = other ? other->getAbstractNode() : nullptr;

        auto cableA = node1->getContacts().at(c1.nodeContact).cable;

        if(!node2)
        {
            // Detach our cable if zero length
            if(cableA && !verifiedCables.contains(cableA))
            {
                auto item = graphForCable(cableA);
                if(!item || item->cableZeroLength())
                {
                    node1->detachCable(c1.nodeContact);
                    removeCable(cableA);
                    cableA = nullptr;
                }
            }

            if(cableA)
                continue; // Already connected

            // There is no other node adjacent to us
            // And we are not connected to a cable
            // Let's see if there is a cable in the next tile
            CableGraphItem *cableGraph = nullptr;
            TileCablePair pair = getCablesAt(otherLocation);
            if(pair.first)
            {
                if(pair.first->sideA() == location)
                {
                    if(pair.first->directionA() == ~c1.direction)
                    {
                        cableGraph = pair.first;
                    }
                }
                else if(pair.first->sideB() == location)
                {
                    if(pair.first->directionB() == ~c1.direction)
                    {
                        cableGraph = pair.first;
                    }
                }
            }

            if(!cableGraph && pair.second)
            {
                if(pair.second->sideA() == location)
                {
                    if(pair.second->directionA() == ~c1.direction)
                    {
                        cableGraph = pair.first;
                    }
                }
                else if(pair.second->sideB() == location)
                {
                    if(pair.second->directionB() == ~c1.direction)
                    {
                        cableGraph = pair.first;
                    }
                }
            }

            // We found a suitable cable, check it
            if(cableGraph)
                checkCable(cableGraph);

            continue;
        }

        std::vector<Connector> otherConnectors;
        other->getConnectors(otherConnectors);

        for(const Connector& c2 : otherConnectors)
        {
            if(c2.direction != ~c1.direction)
                continue;

            // We have a match

            if(!node1 || !node2)
                break;

            connectItems(node1, node2, c1, c2, verifiedCables);
            break;
        }
    }
}

void CircuitScene::addCableTiles(CableGraphItem *item)
{
    if(item->cableZeroLength())
        return;

    for(int i = 0; i < item->cablePath().getTilesCount(); i++)
    {
        TileLocation tile = item->cablePath().at(i);

        auto it = mCableTiles.find(tile);
        if(it == mCableTiles.end())
        {
            TileCablePair pair;
            pair.first = item;
            pair.second = nullptr;
            mCableTiles.insert({tile, pair});
        }
        else
        {
            TileCablePair &pair = it->second;
            if(pair.first)
                pair.second = item;
            else
                pair.first = item;
        }
    }
}

void CircuitScene::removeCableTiles(CableGraphItem *item)
{
    if(item->cableZeroLength())
        return;

    for(int i = 0; i < item->cablePath().getTilesCount(); i++)
    {
        TileLocation tile = item->cablePath().at(i);

        auto it = mCableTiles.find(tile);
        Q_ASSERT(it != mCableTiles.end());

        TileCablePair &pair = it->second;
        Q_ASSERT(pair.first == item || pair.second == item);

        if(pair.first == item)
            pair.first = nullptr;
        else
            pair.second = nullptr;

        if(!pair.first && !pair.second)
            mCableTiles.erase(it); // No more cables on this tile
    }
}

void CircuitScene::editCableUpdatePen()
{
    if(!isEditingCable())
        return;

    QPen pen;
    pen.setWidthF(6.0);

    if(mEditNewCablePath->isComplete())
        pen.setColor(Qt::red);
    else
        pen.setColor(Qt::green);
    mEditNewPath->setPen(pen);
}

AbstractNodeGraphItem *CircuitScene::itemBeingMoved() const
{
    return mItemBeingMoved;
}

void CircuitScene::startMovingItem(AbstractNodeGraphItem *item)
{
    stopUnfinishedOperations();
    mItemBeingMoved = item;
    Q_ASSERT(mItemBeingMoved);

    mLastMovedItemValidLocation = mItemBeingMoved->location();

    if(mMode == Mode::Editing)
        mItemBeingMoved->setFlag(QGraphicsItem::ItemIsMovable, true);
}

void CircuitScene::endMovingItem()
{
    if(!mItemBeingMoved)
        return;

    Q_ASSERT(mLastMovedItemValidLocation.isValid());

    // After move has ended we go back to last valid location
    if(mItemBeingMoved->location() != mLastMovedItemValidLocation)
    {
        mItemBeingMoved->setLocation(mLastMovedItemValidLocation);
    }
    mItemBeingMoved->setFlag(QGraphicsItem::ItemIsMovable, false);

    AbstractNodeGraphItem *item = mItemBeingMoved;
    mItemBeingMoved = nullptr;

    // Since this might add cables and adding cables
    // calls stopOperations() which calls endMovingItem()
    // we call this after resetting mItemBeingMoved
    // This way we prevent recursion
    refreshItemConnections(item, true);
}

void CircuitScene::stopUnfinishedOperations()
{
    endEditCable(false);
    endMovingItem();
}

void CircuitScene::requestEditNode(AbstractNodeGraphItem *item)
{
    stopUnfinishedOperations();

    if(mode() == Mode::Editing)
        emit nodeEditRequested(item);
}

void CircuitScene::requestEditCable(CableGraphItem *item)
{
    stopUnfinishedOperations();

    if(mode() == Mode::Editing)
        emit cableEditRequested(item);
}

void CircuitScene::setRelaisModel(RelaisModel *newRelaisModel)
{
    mRelaisModel = newRelaisModel;
}

void CircuitScene::removeAllItems()
{
    stopUnfinishedOperations();

    // Disable all circuits
    for(PowerSourceGraphItem *powerSource : std::as_const(mPowerSources))
    {
        powerSource->node()->setEnabled(false);
    }

    const auto cableCopy = mCables;
    for(const auto& it : cableCopy)
    {
        CableGraphItem *item = it.second;
        removeCable(item->cable());
    }
    Q_ASSERT(mCables.size() == 0);

    const auto itemsCopy = mItemMap;
    for(const auto& it : itemsCopy)
    {
        AbstractNodeGraphItem *item = it.second;
        removeNode(item);
    }
    Q_ASSERT(mItemMap.size() == 0);
    Q_ASSERT(mPowerSources.size() == 0);
}

bool CircuitScene::loadFromJSON(const QJsonObject &obj, NodeEditFactory *factory)
{
    removeAllItems();

    setMode(Mode::LoadingFile);

    if(!obj.contains("cables") || !obj.contains("nodes"))
        return false;

    const QJsonArray cables = obj.value("cables").toArray();

    for(const QJsonValue& v : cables)
    {
        const QJsonObject cableObj = v.toObject();

        // Create new cable
        CircuitCable *cable = new CircuitCable(this);
        CableGraphItem *item = new CableGraphItem(cable);
        item->setPos(0, 0);

        item->loadFromJSON(cableObj);
        addCable(item);
    }

    const QJsonArray nodes = obj.value("nodes").toArray();

    for(const QJsonValue& v : nodes)
    {
        const QJsonObject obj = v.toObject();
        const QString nodeType = obj.value("type").toString();
        if(nodeType.isEmpty())
            continue;

        // Create new node
        AbstractNodeGraphItem *item = factory->createItem(nodeType, this);
        if(item)
        {
            item->loadFromJSON(obj);
        }
    }

    // Recalculate circuits
    calculateConnections();

    // Turn on power sources and stuff
    setMode(Mode::Simulation);

    return true;
}

void CircuitScene::saveToJSON(QJsonObject &obj) const
{
    QJsonArray cables;
    for(const auto& it : mCables)
    {
        CableGraphItem *item = it.second;

        QJsonObject obj;
        item->saveToJSON(obj);
        cables.append(obj);
    }

    obj["cables"] = cables;

    QJsonArray nodes;
    for(const auto& it : mItemMap)
    {
        AbstractNodeGraphItem *item = it.second;
        QJsonObject obj;
        item->saveToJSON(obj);
        nodes.append(obj);
    }

    obj["nodes"] = nodes;
}

RelaisModel *CircuitScene::relaisModel() const
{
    return mRelaisModel;
}

QPointF CircuitScene::getConnectorPoint(TileLocation l, Connector::Direction direction)
{
    const QPointF pos = l.toPoint();

    switch (direction)
    {
    case Connector::Direction::North:
        return {pos.x() + TileLocation::HalfSize, pos.y()};
    case Connector::Direction::South:
        return {pos.x() + TileLocation::HalfSize, pos.y() + TileLocation::Size};
    case Connector::Direction::East:
        return {pos.x() + TileLocation::Size, pos.y() + TileLocation::HalfSize};
    case Connector::Direction::West:
        return {pos.x(), pos.y() + TileLocation::HalfSize};
    default:
        break;
    }

    return QPointF();
}

void CircuitScene::startEditNewCable()
{
    stopUnfinishedOperations();

    // Cable being edited
    mIsEditingNewCable = true;

    // Overlay to highlight cable
    QPen pen;
    pen.setWidthF(6.0);
    pen.setColor(Qt::blue);
    mEditOverlay = addPath(QPainterPath(), pen);
    mEditOverlay->setZValue(1.0);

    // New cable path, starts empty
    pen.setColor(Qt::green);
    mEditNewPath = addPath(QPainterPath(), pen);
    mEditNewPath->setZValue(2.0);

    // Cable path logic
    mEditNewCablePath = new CableGraphPath;
}

void CircuitScene::startEditCable(CableGraphItem *item)
{
    stopUnfinishedOperations();

    // Cable being edited
    mEditingCable = item;

    // Overlay to highlight cable
    QPen pen;
    pen.setWidthF(6.0);
    pen.setColor(Qt::blue);
    mEditOverlay = addPath(mEditingCable->path(), pen);
    mEditOverlay->setZValue(1.0);

    // New cable path, starts empty
    pen.setColor(Qt::green);
    mEditNewPath = addPath(QPainterPath(), pen);
    mEditNewPath->setZValue(2.0);

    // Cable path logic
    mEditNewCablePath = new CableGraphPath;
}

void CircuitScene::endEditCable(bool apply)
{
    if(!isEditingCable())
        return;

    // Store edit state
    CableGraphPath cablePath = *mEditNewCablePath;
    CableGraphItem *item = mEditingCable;
    bool isNew = mIsEditingNewCable;

    // Reset editing to prevent recursion
    mIsEditingNewCable = false;
    mEditingCable = nullptr;

    delete mEditOverlay;
    mEditOverlay = nullptr;

    delete mEditNewPath;
    mEditNewPath = nullptr;

    delete mEditNewCablePath;
    mEditNewCablePath = nullptr;

    if(!apply)
        return;

    if(!cablePath.isComplete())
        return; // TODO: error message

    if(isNew)
    {
        // Create new cable
        CircuitCable *cable = new CircuitCable(this);
        item = new CableGraphItem(cable);
        item->setPos(0, 0);
    }

    item->setCablePath(cablePath);

    if(isNew)
        addCable(item);

    // Try to connect it right away
    checkCable(item);
}

void CircuitScene::editCableAddPoint(const QPointF &p, bool allowEdge)
{
    int16_t hx = static_cast<int16_t>(std::round(p.x() / TileLocation::HalfSize));
    int16_t hy = static_cast<int16_t>(std::round(p.y() / TileLocation::HalfSize));

    TileLocation location{int16_t(hx / 2), int16_t(hy / 2)};

    QPointF realPoint(hx * TileLocation::HalfSize,
                      hy * TileLocation::HalfSize);

    const bool isEdge = (hy % 2) != (hx % 2);

    Connector::Direction direction = Connector::Direction::North;
    if(std::abs(hy) % 2 == 1)
    {
        if(isEdge)
            direction = Connector::Direction::West;
        location.y = (hy - 1) / 2;
    }
    else
    {
        location.y = hy / 2;
        if(p.y() < realPoint.y())
        {
            location.y--;
            if(isEdge)
                direction = Connector::Direction::South;
        }
    }

    if(std::abs(hx) % 2 == 1)
    {
        location.x = (hx - 1) / 2;
    }
    else
    {
        location.x = hx / 2;
        if(p.x() < realPoint.x())
        {
            location.x--;
            if(isEdge)
                direction = Connector::Direction::East;
        }
    }

    QPointF tileCenter = location.toPoint();
    tileCenter.rx() += TileLocation::HalfSize;
    tileCenter.ry() += TileLocation::HalfSize;

    CableGraphPath newCablePath = *mEditNewCablePath;

    if(newCablePath.isEmpty() && isEdge)
    {
        newCablePath.setStartDirection(direction);
        newCablePath.addTile(location);
    }
    else if(!newCablePath.isEmpty())
    {
        TileLocation lastTile = newCablePath.last();

        const auto deltaX = std::abs(lastTile.x - location.x);
        const auto deltaY = std::abs(lastTile.y - location.y);

        const double maxDiff = 2;

        bool validPoint = true;
        if(deltaY > deltaX && deltaX <= maxDiff && deltaY != 0)
        {
            // Vertical line from last center to new center
            location.x = lastTile.x;
        }
        else if(deltaY < deltaX && deltaY <= maxDiff && deltaX != 0)
        {
            // Horizontal line from last center to new center
            location.y = lastTile.y;
        }
        else
        {
            validPoint = false;
        }

        if(validPoint)
        {
            if(location.x == lastTile.x)
            {
                if(location.y > lastTile.y)
                {
                    for(int16_t y = lastTile.y + 1; y <= location.y; y++)
                    {
                        newCablePath.addTile({location.x, y});
                    }
                }
                else
                {
                    for(int16_t y = lastTile.y - 1; y >= location.y; y--)
                    {
                        newCablePath.addTile({location.x, y});
                    }
                }
            }
            else
            {
                if(location.x > lastTile.x)
                {
                    for(int16_t x = lastTile.x + 1; x <= location.x; x++)
                    {
                        newCablePath.addTile({x, location.y});
                    }
                }
                else
                {
                    for(int16_t x = lastTile.x - 1; x >= location.x; x--)
                    {
                        newCablePath.addTile({x, location.y});
                    }
                }
            }
        }

        if(isEdge && allowEdge && deltaX == 0 && deltaY == 0)
        {
            // Allow edge only if last point was center of same tile
            newCablePath.setEndDirection(direction);
        }
    }

    if(!cablePathIsValid(newCablePath, mEditingCable))
        return;

    // Store new path
    *mEditNewCablePath = newCablePath;

    mEditNewPath->setPath(mEditNewCablePath->generatePath());
    editCableUpdatePen();
}

void CircuitScene::editCableUndoLast()
{
    if(!isEditingCable())
        return;

    mEditNewCablePath->removeLastLine();
    mEditNewPath->setPath(mEditNewCablePath->generatePath());
    editCableUpdatePen();
}

void CircuitScene::keyReleaseEvent(QKeyEvent *e)
{
    if(isEditingCable())
    {
        bool consumed = true;

        switch (e->key())
        {
        case Qt::Key_Escape:
            endEditCable(false);
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            endEditCable(true);
            break;
        default:
            consumed = false;
            break;
        }

        if(!consumed && e->matches(QKeySequence::Undo))
        {
            consumed = true;

            editCableUndoLast();
        }

        if(consumed)
        {
            e->accept();
            return;
        }
    }

    QGraphicsScene::keyReleaseEvent(e);
}

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    if(isEditingCable() && (e->button() == Qt::LeftButton || e->button() == Qt::RightButton))
    {
        const bool allowEdge = e->button() == Qt::RightButton;
        editCableAddPoint(e->scenePos(), allowEdge);
        e->accept();
        return;
    }

    QGraphicsScene::mousePressEvent(e);
}

void CircuitScene::refreshItemConnections(AbstractNodeGraphItem *item, bool tryReconnect)
{
    // Detach all contacts, will be revaluated later
    AbstractCircuitNode *node = item->getAbstractNode();
    const auto& contacts = node->getContacts();

    for(int i = 0; i < contacts.size(); i++)
    {
        CircuitCable *cable = contacts.at(i).cable;
        node->detachCable(i);

        if(cable)
        {
            // Delete zero length cables
            // This way also opposite node becomes unconnected
            // And can receive future connections
            // Even before ending Editing mode
            auto *cableGraph = graphForCable(cable);
            if(cableGraph && cableGraph->cableZeroLength())
            {
                removeCable(cable);
            }
        }

    }

    if(!tryReconnect)
        return;

    // Try reconnect item
    QVector<CircuitCable *> dummy;
    checkItem(item, dummy);
}
