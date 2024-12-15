/**
 * src/circuits/circuitscene.cpp
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

#include "view/circuitlistmodel.h"
#include "../views/modemanager.h"

#include "graphs/abstractnodegraphitem.h"
#include "graphs/cablegraphitem.h"
#include "nodes/circuitcable.h"

#include "graphs/powersourcegraphitem.h"
#include "nodes/powersourcenode.h"

#include <QGraphicsPathItem>
#include <QPen>

#include <unordered_set>

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QPainter>

#include <QClipboard>
#include <QMimeData>

#include "edit/nodeeditfactory.h"

template <typename Func>
bool spiral_helper(const TileLocation& origin,
                   TileLocation &outTile,
                   Func func)
{
    // Loop in a square spiral around origin
    // For each tile, func is called
    // If func returns true, looping is stopped
    const int N = 1000;
    int16_t x = 0;
    int16_t y = 0;
    for(int i = 0; i < N; ++i)
    {
        const TileLocation tile = origin.adjusted(x, y);
        if(func(tile))
        {
            outTile = tile;
            return true;
        }

        if(std::abs(x) <= std::abs(y) && (x != y || x >= 0))
            x += ((y >= 0) ? 1 : -1);
        else
            y += ((x >= 0) ? -1 : 1);
    }

    return false;
}

template <typename TileHasNode, typename GetCablePairAt>
bool cablePathIsValid_helper(const CableGraphPath &cablePath,
                             TileHasNode hasNode, GetCablePairAt getCablePairAt)
{
    // Check all tiles are free
    if(cablePath.isZeroLength())
        return true;

    std::unordered_set<TileLocation, TileLocationHash> repeatedTiles;

    for(int i = 0; i < cablePath.getTilesCount(); i++)
    {
        TileLocation tile = cablePath.at(i);

        if(hasNode(tile))
        {
            return false;
        }

        CircuitScene::TileCablePathPair pair = getCablePairAt(tile);

        if(!pair.first && !pair.second)
        {
            // Tile is free, check next one
            repeatedTiles.insert(tile);
            continue;
        }

        if(pair.first && pair.second)
        {
            // There are already 2 cables on this tile
            return false;
        }

        // At this point tile has 1 cable, either first or second
        if(repeatedTiles.find(tile) != repeatedTiles.cend())
        {
            // Cable passes 2 times on same tile
            // Then this tile cannot have other cables
            return false;
        }
        repeatedTiles.insert(tile);

        // Check if other cable can co-exist with us in same tile
        const CableGraphPath& otherPath = pair.first ? pair.first.value() : pair.second.value();
        int otherIdx = otherPath.tiles().indexOf(tile);
        Q_ASSERT_X(otherIdx >= 0,
                   "cablePathIsValid_helper",
                   "other cable does not contain tile");

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

    return true;
}

CircuitScene::CircuitScene(CircuitListModel *parent)
    : QGraphicsScene{parent}
{

}

CircuitScene::~CircuitScene()
{
    removeAllItems();
}

FileMode CircuitScene::mode() const
{
    return circuitsModel()->modeMgr()->mode();
}

ModeManager *CircuitScene::modeMgr() const
{
    return circuitsModel()->modeMgr();
}

void CircuitScene::setMode(FileMode newMode, FileMode oldMode)
{
    if(oldMode == FileMode::Editing)
    {
        // Recalculate circuit
        calculateConnections();
    }

    const bool powerSourceEnabled = newMode == FileMode::Simulation;
    for(PowerSourceGraphItem *powerSource : std::as_const(mPowerSources))
    {
        powerSource->node()->setEnabled(powerSourceEnabled);
    }

    // Background changes between modes
    invalidate(QRectF(), BackgroundLayer);
}

void CircuitScene::addNode(AbstractNodeGraphItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    if(!isLocationFree(item->location()))
    {
        // Assign a new location to node
        item->setLocation(getFreeLocationNear(item->location()));
    }

    mItemMap.insert({item->location(), item});

    // Add item after having inserted it in the map
    addItem(item);

    PowerSourceGraphItem *powerSource = qobject_cast<PowerSourceGraphItem *>(item);
    if(powerSource)
    {
        powerSource->node()->setEnabled(false);
        mPowerSources.append(powerSource);
    }

    setHasUnsavedChanges(true);
}

void CircuitScene::removeNode(AbstractNodeGraphItem *item)
{
    if(item == itemBeingMoved())
        endMovingItem();

    Q_ASSERT_X(getNodeAt(item->location()) == item,
               "removeNode", "item location is not in item map");

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

    setHasUnsavedChanges(true);
}

void CircuitScene::addCable(CableGraphItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    Q_ASSERT(item->cable());

    if(!cablePathIsValid(item->cablePath(), nullptr))
        return; // TODO: error

    addItem(item);
    mCables.insert({item->cable(), item});

    // Add cable tiles
    addCableTiles(item);

    setHasUnsavedChanges(true);
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

    setHasUnsavedChanges(true);
}

CableGraphItem *CircuitScene::graphForCable(CircuitCable *cable) const
{
    auto it = mCables.find(cable);
    if(it == mCables.end())
        return nullptr;
    return it->second;
}

TileLocation CircuitScene::getFreeLocationNear(TileLocation origin)
{
    if(!origin.isValid())
        origin = {0, 0};

    TileLocation result = TileLocation::invalid;
    bool success = spiral_helper(origin, result, [this](const TileLocation& tile)
    {
        return isLocationFree(tile);
    });

    if(success)
        return result;

    return TileLocation::invalid;
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
    auto hasNode = [this](const TileLocation& tile) -> bool
    {
        return getItemAt(tile) != nullptr;
    };

    auto getCablePairAt = [this, item](const TileLocation& tile) -> TileCablePathPair
    {
        const TileCablePair cablePair = getCablesAt(tile);

        TileCablePathPair pathPair;

        // Do not consider ourselves
        if(cablePair.first && cablePair.first != item)
            pathPair.first = cablePair.first->cablePath();
        if(cablePair.second && cablePair.second != item)
            pathPair.second = cablePair.second->cablePath();

        return pathPair;
    };

    return cablePathIsValid_helper(cablePath, hasNode, getCablePairAt);
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

    Q_ASSERT_X(getItemAt(oldLocation) == item,
               "updateItemLocation", "Item old location is not in map");

    // Update location in map
    mItemMap.erase(oldLocation);
    mItemMap.insert({newLocation, item});

    setHasUnsavedChanges(true);

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

    // Delete unconnected zero length cables and empty cables
    // Copy to avoid invalidating iterators while looping
    cableCopy = mCables;
    for(auto it = cableCopy.begin(); it != cableCopy.end(); it++)
    {
        CableGraphItem *item = it->second;
        if(item->cablePath().isEmpty() || (item->cableZeroLength() && !verifiedCables.contains(item->cable())))
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

            CircuitCable *newCable = new CircuitCable(circuitsModel()->modeMgr(), this);

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

    if(!item->cableZeroLength())
    {
        // If there is no node next to us,
        // check if there are other cables to merge with
        if(!itemA)
        {
            const TileCablePair pair = getCablesAt(nodeLocA);

            CableGraphPath merged;
            CableGraphItem *other = nullptr;

            if(pair.first)
            {
                merged = item->cablePath().tryMerge(pair.first->cablePath());
                other = pair.first;
            }
            if(merged.isEmpty() && pair.second)
            {
                merged = item->cablePath().tryMerge(pair.second->cablePath());
                other = pair.second;
            }

            if(!merged.isEmpty())
            {
                // We cannot delete other cable because
                // this function is called iterating cable list
                // so we set it to empty path and it will be garbage collected
                other->setCablePath(CableGraphPath{});

                // Set our path to merged
                item->setCablePath(merged);

                // Retry searching nodes
                nodeLocA = item->sideA();
                itemA = getItemAt(nodeLocA);
            }
        }

        if(!itemB)
        {
            const TileCablePair pair = getCablesAt(nodeLocB);

            CableGraphPath merged;
            CableGraphItem *other = nullptr;

            if(pair.first)
            {
                merged = item->cablePath().tryMerge(pair.first->cablePath());
                other = pair.first;
            }
            if(merged.isEmpty() && pair.second)
            {
                merged = item->cablePath().tryMerge(pair.second->cablePath());
                other = pair.second;
            }

            if(!merged.isEmpty())
            {
                // We cannot delete other cable because
                // this function is called iterating cable list
                // so we set it to empty path and it will be garbage collected
                other->setCablePath(CableGraphPath{});

                // Set our path to merged
                item->setCablePath(merged);

                // Retry searching nodes
                nodeLocB = item->sideB();
                itemB = getItemAt(nodeLocB);
            }
        }
    }

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
                auto cableGraphA = graphForCable(cableA);
                if(!cableGraphA || cableGraphA->cableZeroLength())
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

bool CircuitScene::splitCableAt(CableGraphItem *item, const TileLocation& splitLoc)
{
    CableGraphPath::SplitPair pathResult;
    if(!item->cablePath().splitted(splitLoc, pathResult))
        return false;

    // Set original cable path to new splitted half
    bool firstEmpty = pathResult.first.isEmpty();
    bool secondEmpty = pathResult.second.isEmpty();

    if(firstEmpty && secondEmpty)
    {
        // Cable was only 1 tile long, remove it
        removeCable(item->cable());
        return true;
    }

    // Set original cable to non-empty half
    item->setCablePath(firstEmpty ? pathResult.second : pathResult.first);
    checkCable(item);

    if(firstEmpty)
        return true; // Only one half was left

    // Create new cable for second half
    CircuitCable *otherCable = new CircuitCable(circuitsModel()->modeMgr(), this);
    CableGraphItem *otherItem = new CableGraphItem(otherCable);
    otherItem->setPos(0, 0);

    otherItem->setCablePath(pathResult.second);

    // Register and check new cable
    addCable(otherItem);
    checkCable(otherItem);

    return true;
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
        Q_ASSERT_X(it != mCableTiles.end(),
                   "removeCableTiles", "unknown tile");

        TileCablePair &pair = it->second;
        Q_ASSERT_X(pair.first == item || pair.second == item,
                   "removeCableTiles", "cable not registered");

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
    modeMgr()->setEditingSubMode(EditingSubMode::SingleItemMove);

    mItemBeingMoved = item;
    Q_ASSERT(mItemBeingMoved);

    mLastMovedItemValidLocation = mItemBeingMoved->location();
    mMovedItemOriginalLocation = mLastMovedItemValidLocation;

    if(circuitsModel()->modeMgr()->mode() == FileMode::Editing)
        mItemBeingMoved->setFlag(QGraphicsItem::ItemIsMovable, true);
}

void CircuitScene::endMovingItem()
{
    if(!mItemBeingMoved)
        return;

    Q_ASSERT_X(mLastMovedItemValidLocation.isValid(),
               "endMovingItem", "last location not valid");

    // After move has ended we go back to last valid location
    const TileLocation possibleNewLocation = mItemBeingMoved->location();
    const TileLocation lastValidLocation = mLastMovedItemValidLocation;
    if(possibleNewLocation != lastValidLocation)
    {
        mItemBeingMoved->setLocation(lastValidLocation);
    }
    mItemBeingMoved->setFlag(QGraphicsItem::ItemIsMovable, false);

    const bool itemReallyMoved = (possibleNewLocation != mMovedItemOriginalLocation);
    AbstractNodeGraphItem *item = mItemBeingMoved;
    mItemBeingMoved = nullptr;
    mLastMovedItemValidLocation = TileLocation::invalid;
    mMovedItemOriginalLocation = mLastMovedItemValidLocation;

    // Since this might add cables and adding cables
    // calls stopOperations() which calls endMovingItem()
    // we call this after resetting mItemBeingMoved
    // This way we prevent recursion
    if(modeMgr()->editingSubMode() == EditingSubMode::SingleItemMove)
        modeMgr()->setEditingSubMode(EditingSubMode::Default);

    const bool shiftPressed = QGuiApplication::keyboardModifiers() == Qt::ShiftModifier;
    if(shiftPressed && possibleNewLocation != lastValidLocation)
    {
        // If shift is pressed and node is on a cable, split the cable
        auto otherNode = getNodeAt(possibleNewLocation);
        TileCablePair pair;

        bool canSplitCables = true;

        if(otherNode)
        {
            // We are over another node
            // This is not valid, return to last valid position
            canSplitCables = false;
        }

        if(canSplitCables)
        {
            pair = getCablesAt(possibleNewLocation);

            if(pair.first)
            {
                if(!splitCableAt(pair.first, possibleNewLocation))
                    canSplitCables = false;
            }
        }

        if(canSplitCables && pair.second)
        {
            if(!splitCableAt(pair.second, possibleNewLocation))
                canSplitCables = false;
        }

        if(canSplitCables)
        {
            // We succeded in splitting existing cables
            // Place node in previously invalid location
            item->setLocation(possibleNewLocation);
        }
    }

    // This might trigger cable refresh which results in file
    // having unsaved changes and asking user to save.
    // So do it only if really moved
    if(itemReallyMoved)
        refreshItemConnections(item, true);
}

void CircuitScene::requestEditNode(AbstractNodeGraphItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    if(mode() == FileMode::Editing)
        emit circuitsModel()->nodeEditRequested(item);
}

void CircuitScene::requestEditCable(CableGraphItem *item)
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    if(mode() == FileMode::Editing)
        emit circuitsModel()->cableEditRequested(item);
}

void CircuitScene::onEditingSubModeChanged(EditingSubMode oldMode, EditingSubMode newMode)
{
    switch (oldMode)
    {
    case EditingSubMode::CableEditing:
        endEditCable(false);
        break;
    case EditingSubMode::SingleItemMove:
        endMovingItem();
        break;
    case EditingSubMode::ItemSelection:
        endItemSelection();
        break;
    case EditingSubMode::Default:
    default:
        break;
    }

    switch (newMode)
    {
    case EditingSubMode::ItemSelection:
        startItemSelection();
        break;
    default:
        break;
    }
}

void CircuitScene::startItemSelection()
{
    modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);

    allowItemSelection(true);
}

void CircuitScene::endItemSelection()
{
    endSelectionMove();

    clearSelection();

    allowItemSelection(false);

    mSelectedItemPositions.clear();
    mSelectedCablePositions.clear();

    // Disable item selection if not already done
    if(modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
        modeMgr()->setEditingSubMode(EditingSubMode::Default);
}

void CircuitScene::allowItemSelection(bool enabled)
{
    for(const auto& it : mCables)
    {
        CableGraphItem *item = it.second;
        item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    }

    for(const auto& it : mItemMap)
    {
        AbstractNodeGraphItem *item = it.second;
        item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    }
}

void CircuitScene::onItemSelected(AbstractNodeGraphItem *item, bool value)
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    if(value)
    {
        const TileLocation tile = item->location();
        Q_ASSERT_X(getItemAt(tile) == item, "onItemSelected",
                   "Item location is not registered");

        mSelectedItemPositions.insert({item, tile});
    }
    else
    {
        auto it = mSelectedItemPositions.find(item);
        if(it != mSelectedItemPositions.end())
        {
            // Reset to last valid location
            const TileLocation lastValidLocation = it->second;
            item->setLocation(lastValidLocation);
            mSelectedItemPositions.erase(it);
        }
    }
}

void CircuitScene::onCableSelected(CableGraphItem *item, bool value)
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    if(item->cableZeroLength())
        return;

    if(value)
    {
        const TileLocation firstTile = item->cablePath().first();
        mSelectedCablePositions.insert({item, {firstTile, firstTile}});
    }
    else
    {
        auto it = mSelectedCablePositions.find(item);
        if(it != mSelectedCablePositions.end())
        {
            // Reset to last valid location
            const TileLocation lastValidLocation = it->second.first;
            const TileLocation firstTile = item->cablePath().first();

            if(lastValidLocation != firstTile)
            {
                const int16_t dx = lastValidLocation.x - firstTile.x;
                const int16_t dy = lastValidLocation.y - firstTile.y;
                CableGraphPath translated = item->cablePath().translatedBy(dx, dy);
                item->setCablePath(translated);
            }
            mSelectedCablePositions.erase(it);
        }
    }
}

void CircuitScene::moveSelectionBy(int16_t dx, int16_t dy)
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    bool allFree = true;

    for(auto it = mSelectedItemPositions.begin();
        it != mSelectedItemPositions.end();
        it++)
    {
        AbstractNodeGraphItem *item = it->first;
        const TileLocation newTile = item->location().adjusted(dx, dy);
        item->setLocation(newTile);

        Q_ASSERT_X(getNodeAt(it->second) == item,
                   "moveSelectionBy", "item last valid location is not in item map");

        if(allFree)
        {
            AbstractNodeGraphItem *otherItem = getItemAt(newTile);
            if(otherItem &&
                    mSelectedItemPositions.find(otherItem) == mSelectedItemPositions.end())
            {
                // We landed on top of another item which is not being moved
                allFree = false;
            }
        }

        if(allFree)
        {
            // Check if move is valid
            TileCablePair pair = getCablesAt(newTile);
            if(pair.first)
            {
                // Check if we landed on top of a cable which is not being moved
                if(mSelectedCablePositions.find(pair.first) == mSelectedCablePositions.end())
                    allFree = false;
            }
            if(allFree && pair.second)
            {
                // Check if we landed on top of a cable which is not being moved
                if(mSelectedCablePositions.find(pair.second) == mSelectedCablePositions.end())
                    allFree = false;
            }
        }
    }

    for(auto it = mSelectedCablePositions.begin();
        it != mSelectedCablePositions.end();
        it++)
    {
        CableGraphItem *item = it->first;
        const TileLocation lastValidLocation = it->second.first;
        const TileLocation currentFirstLocation = it->second.second;
        const TileLocation newFirstLocation = currentFirstLocation.adjusted(dx, dy);

        const int16_t cableDx = newFirstLocation.x - lastValidLocation.x;
        const int16_t cableDy = newFirstLocation.y - lastValidLocation.y;

        CableGraphPath translated = item->cablePath().translatedBy(cableDx, cableDy);

        // We are not sure yet this is a valid move
        item->setPathInternal(translated.generatePath());

        // Store current possible first location
        it->second.second = newFirstLocation;

        if(allFree)
        {
            // Check if move is valid
            auto hasNode = [this](const TileLocation& tile) -> bool
            {
                AbstractNodeGraphItem *otherItem = getItemAt(tile);
                if(otherItem &&
                        mSelectedItemPositions.find(otherItem) == mSelectedItemPositions.end())
                {
                    // We landed on top of another item which is not being moved
                    return true;
                }

                return false;
            };

            auto getCablePairAt = [this, item](const TileLocation& tile) -> TileCablePathPair
            {
                // Check if we landed on top of a cable which is not being moved
                const TileCablePair cablePair = getCablesAt(tile);

                TileCablePathPair pathPair;

                // Do not consider ourselves
                if(cablePair.first && cablePair.first != item)
                {
                    if(mSelectedCablePositions.find(cablePair.first) == mSelectedCablePositions.end())
                    {
                        // This cable is not being moved with us
                        pathPair.first = cablePair.first->cablePath();
                    }
                }
                if(cablePair.second && cablePair.second != item)
                {
                    if(mSelectedCablePositions.find(cablePair.second) == mSelectedCablePositions.end())
                    {
                        // This cable is not being moved with us
                        pathPair.second = cablePair.second->cablePath();
                    }
                }

                return pathPair;
            };

            if(!cablePathIsValid_helper(translated, hasNode, getCablePairAt))
                allFree = false;
        }
    }

    if(allFree)
    {
        // Unregister last valid position
        // This is because items could move to position previously held
        // by other moved items, so they could block each other
        for(auto it = mSelectedItemPositions.begin();
            it != mSelectedItemPositions.end();
            it++)
        {
            AbstractNodeGraphItem *item = it->first;
            const TileLocation oldLocation = it->second;

            Q_ASSERT_X(getNodeAt(oldLocation) == item,
                       "moveSelectionBy", "item OLD location is not in item map");

            mItemMap.erase(oldLocation);
        }

        // Register new valid position for all nodes
        for(auto it = mSelectedItemPositions.begin();
            it != mSelectedItemPositions.end();
            it++)
        {
            AbstractNodeGraphItem *item = it->first;
            const TileLocation newLocation = item->location();

            Q_ASSERT_X(getNodeAt(newLocation) == nullptr,
                       "moveSelectionBy", "item being moved to invalid position");

            // Update location in map
            mItemMap.insert({newLocation, item});

            // Save last valid location
            it->second = newLocation;

            Q_ASSERT_X(getNodeAt(newLocation) == item,
                       "moveSelectionBy", "item new location is not in item map");
            Q_ASSERT_X(item->location() == newLocation,
                       "moveSelectionBy", "error");

            Q_ASSERT_X(getNodeAt(item->location()) == item,
                       "moveSelectionBy", "item new location is not in item map");
        }

        // Unregister old tiles for all cables
        // So that during move they do not conflict with each other
        for(auto it = mSelectedCablePositions.begin();
            it != mSelectedCablePositions.end();
            it++)
        {
            CableGraphItem *item = it->first;
            removeCableTiles(item);
        }

        // Register new valid position for all cables
        for(auto it = mSelectedCablePositions.begin();
            it != mSelectedCablePositions.end();
            it++)
        {
            CableGraphItem *item = it->first;
            const TileLocation lastValidLocation = it->second.first;
            const TileLocation currentFirstLocation = it->second.second;

            const int16_t cableDx = currentFirstLocation.x - lastValidLocation.x;
            const int16_t cableDy = currentFirstLocation.y - lastValidLocation.y;

            const CableGraphPath translated = item->cablePath().translatedBy(cableDx, cableDy);

            // Now we really apply path
            item->setCablePathInternal(translated, false);

            // And register new tiles
            addCableTiles(item);

            // Store new valid location to current first location
            it->second.first = currentFirstLocation;
        }

        setHasUnsavedChanges(true);
    }
}

void CircuitScene::endSelectionMove()
{
    mSelectedCableMoveStart = TileLocation::invalid;

    // Reset all selected nodes to last valid location
    for(auto it = mSelectedItemPositions.begin();
        it != mSelectedItemPositions.end();
        it++)
    {
        AbstractNodeGraphItem *item = it->first;
        const TileLocation lastValidLocation = it->second;
        item->setLocation(lastValidLocation);

        Q_ASSERT_X(getNodeAt(item->location()) == item,
                   "endSelectionMove", "item location is not in item map");
    }

    // Reset all selected cables to last valid location
    for(auto it = mSelectedCablePositions.begin();
        it != mSelectedCablePositions.end();
        it++)
    {
        CableGraphItem *item = it->first;
        const TileLocation lastValidLocation = it->second.first;
        const TileLocation currentFirstLocation = it->second.second;

        if(lastValidLocation != currentFirstLocation)
        {
            // Reset to last valid path
            item->setCablePath(item->cablePath());

            // Set new first to last valid location
            it->second.second = lastValidLocation;
        }
    }
}

void CircuitScene::moveSelectedCableAt(const TileLocation &tile)
{
    if(!tile.isValid())
        return;

    if(!mSelectedCableMoveStart.isValid())
    {
        // Start move, save original mouse position
        mSelectedCableMoveStart = tile;
        return;
    }

    if(mSelectedCableMoveStart == tile)
        return; // No move

    int16_t dx = tile.x - mSelectedCableMoveStart.x;
    int16_t dy = tile.y - mSelectedCableMoveStart.y;

    // Update new move start to track relative mouse moves
    mSelectedCableMoveStart = tile;

    moveSelectionBy(dx, dy);
}

void CircuitScene::copySelectedItems()
{
    QJsonArray nodes;
    for(auto it : mSelectedItemPositions)
    {
        AbstractNodeGraphItem *item = it.first;

        QJsonObject nodeObj;
        item->saveToJSON(nodeObj);
        nodes.append(nodeObj);
    }

    QJsonArray cables;
    for(auto it : mSelectedCablePositions)
    {
        CableGraphItem *item = it.first;

        QJsonObject cableObj;
        item->saveToJSON(cableObj);
        cables.append(cableObj);
    }

    QJsonObject rootObj;
    rootObj["nodes"] = nodes;
    rootObj["cables"] = cables;

    QByteArray value = QJsonDocument(rootObj).toJson(QJsonDocument::Compact);

    QMimeData *mime = new QMimeData;
    mime->setData(CircuitMimeType, value);

    QGuiApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

bool CircuitScene::tryPasteItems(const TileLocation &tileHint,
                                 TileLocation &outTopLeft,
                                 TileLocation &outBottomRight)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if(!clipboard)
        return false;

    const QMimeData *mime = clipboard->mimeData(QClipboard::Clipboard);
    if(!mime)
        return false;

    QByteArray value = mime->data(CircuitMimeType);
    if(value.isEmpty())
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(value);
    if(doc.isNull())
        return false;


    const QJsonObject rootObj = doc.object();

    return insertFragment(tileHint, rootObj,
                          modeMgr()->circuitFactory(),
                          outTopLeft, outBottomRight);
}

void CircuitScene::removeSelectedItems()
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    // Copy current selection
    const auto selectedCablesCopy = mSelectedCablePositions;
    const auto selectedItemsCopy = mSelectedItemPositions;

    // Clear selection
    clearSelection();
    mSelectedCablePositions.clear();
    mSelectedItemPositions.clear();

    // Remove previously selected items
    for(auto it : selectedCablesCopy)
    {
        CableGraphItem *item = it.first;
        removeCable(item->cable());
    }

    for(auto it : selectedItemsCopy)
    {
        AbstractNodeGraphItem *item = it.first;
        removeNode(item);
    }
}

void CircuitScene::selectAll()
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    for(const auto& it : mCables)
    {
        CableGraphItem *item = it.second;
        item->setSelected(true);
    }

    for(const auto& it : mItemMap)
    {
        AbstractNodeGraphItem *item = it.second;
        item->setSelected(true);
    }
}

void CircuitScene::invertSelection()
{
    if(modeMgr()->editingSubMode() != EditingSubMode::ItemSelection)
        return;

    const auto oldSelectionCopy = selectedItems();

    selectAll();

    // Now deselect previous items
    for(auto *item : oldSelectionCopy)
    {
        item->setSelected(false);
    }
}

bool CircuitScene::insertFragment(const TileLocation &tileHint,
                                  const QJsonObject &fragmentRoot,
                                  NodeEditFactory *factory,
                                  TileLocation &outTopLeft,
                                  TileLocation &outBottomRight)
{
    const QJsonArray nodes = fragmentRoot.value("nodes").toArray();
    const QJsonArray cables = fragmentRoot.value("cables").toArray();

    // Remove overlapping nodes/cables in fragment to be inserted
    FragmentData fragment;
    if(!checkFragment(nodes, cables, fragment))
        return false;

    // We have now skipped invalid items, check if anything is left
    if(fragment.validNodes.isEmpty() && fragment.validCables.isEmpty())
        return false;

    // Find where to inster fragment, near provided tile hint
    TileLocation fragmentOrigin = TileLocation::invalid;
    if(!locateSpaceForFragment(fragment, tileHint, fragmentOrigin))
        return false;

    // Fragment coordinates are still what use to be when copied
    // So they need to be adjusted
    // Calculate translation to apply to fragmen
    const int16_t dx = fragmentOrigin.x - fragment.topLeftLocation.x;
    const int16_t dy = fragmentOrigin.y - fragment.topLeftLocation.y;

    // Ensure we are default mode (clears previous selection)
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    // Really paste items
    QVector<AbstractNodeGraphItem *> pastedItems;
    pastedItems.reserve(fragment.validNodes.size());

    QVector<CableGraphItem *> pastedCables;
    pastedCables.reserve(fragment.validCables.size());

    for(const QJsonObject& obj : fragment.validNodes)
    {
        const QString nodeType = obj.value("type").toString();
        if(nodeType.isEmpty())
            continue;

        // Create new node
        AbstractNodeGraphItem *item = factory->createItem(nodeType, this);
        if(item)
        {
            if(!item->loadFromJSON(obj))
            {
                auto *node = item->getAbstractNode();
                delete item;
                delete node;
                continue;
            }

            // Translate node
            TileLocation tile = item->location();
            tile = tile.adjusted(dx, dy);
            item->setLocation(tile);

            // Add node to scene
            addNode(item);

            pastedItems.append(item);
        }
    }

    for(const QJsonObject& cableObj : fragment.validCables)
    {
        // Create new cable
        CircuitCable *cable = new CircuitCable(circuitsModel()->modeMgr(), this);
        CableGraphItem *item = new CableGraphItem(cable);
        item->setPos(0, 0);

        if(!item->loadFromJSON(cableObj))
        {
            delete item;
            delete cable;
            continue;
        }

        // Translate cable
        const CableGraphPath translated = item->cablePath().translatedBy(dx, dy);
        item->setCablePath(translated);

        addCable(item);

        pastedCables.append(item);
    }

    // Try to connect new nodes and cables
    QVector<CircuitCable *> verifiedCables;
    for(AbstractNodeGraphItem *item : std::as_const(pastedItems))
    {
        checkItem(item, verifiedCables);
    }

    for(CableGraphItem *item : std::as_const(pastedCables))
    {
        checkCable(item);
    }

    // Now select all pasted items so user can move them
    modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);

    for(QGraphicsItem *item : std::as_const(pastedItems))
    {
        item->setSelected(true);
    }

    for(QGraphicsItem *item : std::as_const(pastedCables))
    {
        item->setSelected(true);
    }

    outTopLeft = fragment.topLeftLocation.adjusted(dx, dy);;
    outBottomRight = fragment.bottomRightLocation.adjusted(dx, dy);

    return true;
}

bool CircuitScene::checkFragment(const QJsonArray &nodes, const QJsonArray &cables, FragmentData &fragment)
{
    fragment.validNodes.reserve(nodes.size());
    fragment.pastedNodeTiles.reserve(nodes.size());

    fragment.validCables.reserve(cables.size());
    fragment.cablePathVec.reserve(cables.size());

    // Check pasted nodes do not overlap each other
    for(const QJsonValue& v : nodes)
    {
        const QJsonObject obj = v.toObject();
        const QString nodeType = obj.value("type").toString();
        if(nodeType.isEmpty())
            continue;

        TileLocation tile{0, 0};
        tile.x = obj.value("x").toInt();
        tile.y = obj.value("y").toInt();

        if(fragment.pastedNodeTiles.contains(tile))
            continue; // Node overlaps another node

        fragment.validNodes.append(obj);
        fragment.pastedNodeTiles.append(tile);

        fragment.trackFragmentBounds(tile);
    }

    // Check pasted cables do not overlap each other or nodes
    typedef std::pair<int, int> PathIdxPair;
    typedef std::unordered_map<TileLocation, PathIdxPair, TileLocationHash> PathPairMap;

    PathPairMap pathPairMap;

    auto hasPastedNode = [&fragment](const TileLocation& tile) -> bool
    {
        return fragment.pastedNodeTiles.contains(tile);
    };

    auto getPastedCablePairAt = [&pathPairMap, &fragment](const TileLocation& tile) -> TileCablePathPair
    {
        // Check if we landed on top of a cable which is being pasted
        auto it = pathPairMap.find(tile);
        if(it == pathPairMap.cend())
            return {};

        const PathIdxPair idxPair = it->second;

        TileCablePathPair pathPair;

        // Do not consider ourselves
        if(idxPair.first >= 0)
        {
            pathPair.first = fragment.cablePathVec.at(idxPair.first);
        }
        if(idxPair.second >= 0)
        {
            pathPair.second = fragment.cablePathVec.at(idxPair.second);
        }

        return pathPair;
    };

    for(const QJsonValue& v : cables)
    {
        const QJsonObject obj = v.toObject();

        const QJsonObject pathObj = obj.value("path").toObject();
        if(pathObj.isEmpty())
            continue;

        CableGraphPath path = CableGraphPath::loadFromJSON(pathObj);
        if(path.isEmpty())
            continue;

        if(!cablePathIsValid_helper(path, hasPastedNode, getPastedCablePairAt))
            return false;

        fragment.validCables.append(obj);

        int idx = fragment.cablePathVec.size();
        fragment.cablePathVec.append(path);

        // Add tiles
        for(const TileLocation& tile : path.tiles())
        {
            auto it = pathPairMap.find(tile);
            if(it == pathPairMap.end())
            {
                PathIdxPair pair;
                pair.first = idx;
                pair.second = -1;
                pathPairMap.insert({tile, pair});
            }
            else
            {
                PathIdxPair &pair = it->second;
                if(pair.first >= 0)
                    pair.second = idx;
                else
                    pair.first = idx;
            }

            fragment.trackFragmentBounds(tile);
        }
    }

    return true;
}

bool CircuitScene::locateSpaceForFragment(const FragmentData &fragment, const TileLocation &tileHint, TileLocation &result)
{
    if(!fragment.topLeftLocation.isValid())
        return false;

    auto hasExistingNode = [this](const TileLocation& tile) -> bool
    {
        return getItemAt(tile) != nullptr;
    };

    auto getExistingCablePairAt = [this](const TileLocation& tile) -> TileCablePathPair
    {
        const TileCablePair cablePair = getCablesAt(tile);

        TileCablePathPair pathPair;

        // Do not consider ourselves
        if(cablePair.first)
            pathPair.first = cablePair.first->cablePath();
        if(cablePair.second)
            pathPair.second = cablePair.second->cablePath();

        return pathPair;
    };

    auto isLocationValid =
            [this, &fragment, hasExistingNode, getExistingCablePairAt](const TileLocation& origin) -> bool
    {
        // Make top left item end on origin
        const TileLocation topLeft = origin.adjusted(-fragment.topLeftLocation.x,
                                                     -fragment.topLeftLocation.y);

        // Check if we paste in origin what happens
        for(const TileLocation& nodeTile : fragment.pastedNodeTiles)
        {
            const TileLocation destTile = nodeTile.adjusted(topLeft.x, topLeft.y);
            if(getNodeAt(destTile))
                return false; // We overlap existing node

            TileCablePair pair = getCablesAt(destTile);
            if(pair.first || pair.second)
                return false; // We overlap an existing cable
        }

        for(const CableGraphPath& path : fragment.cablePathVec)
        {
            const CableGraphPath translated = path.translatedBy(topLeft.x, topLeft.y);

            if(!cablePathIsValid_helper(translated, hasExistingNode, getExistingCablePairAt))
                return false;
        }

        return true;
    };

    return spiral_helper(tileHint, result, isLocationValid);
}

QString CircuitScene::circuitSheetLongName() const
{
    return mCircuitSheetLongName;
}

void CircuitScene::setCircuitSheetLongName(const QString &newLongName)
{
    const QString trimmedName = newLongName.trimmed();

    if(mCircuitSheetLongName == trimmedName)
        return;

    mCircuitSheetLongName = trimmedName;
    emit longNameChanged(mCircuitSheetName, this);

    setHasUnsavedChanges(true);
}

QString CircuitScene::circuitSheetName() const
{
    return mCircuitSheetName;
}

bool CircuitScene::setCircuitSheetName(const QString &newName)
{
    const QString trimmedName = newName.trimmed();

    if(mCircuitSheetName == trimmedName)
        return true;

    bool isValid = circuitsModel()->isNameAvailable(trimmedName);

    if(isValid)
    {
        // Do change name
        mCircuitSheetName = trimmedName;

        setHasUnsavedChanges(true);
    }

    emit nameChanged(mCircuitSheetName, this);

    return isValid;
}

bool CircuitScene::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
}

void CircuitScene::setHasUnsavedChanges(bool newHasUnsavedChanged)
{
    if(m_hasUnsavedChanges == newHasUnsavedChanged)
        return;

    m_hasUnsavedChanges = newHasUnsavedChanged;

    if(m_hasUnsavedChanges)
        circuitsModel()->onSceneEdited();

    emit sceneEdited(m_hasUnsavedChanges);
}

void CircuitScene::removeAllItems()
{
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

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

    if(!obj.contains("cables") || !obj.contains("nodes"))
        return false;

    if(!setCircuitSheetName(obj.value("name").toString()))
        return false;

    setCircuitSheetLongName(obj.value("long_name").toString());

    const QJsonArray cables = obj.value("cables").toArray();

    for(const QJsonValue& v : cables)
    {
        const QJsonObject cableObj = v.toObject();

        // Create new cable
        CircuitCable *cable = new CircuitCable(circuitsModel()->modeMgr(), this);
        CableGraphItem *item = new CableGraphItem(cable);
        item->setPos(0, 0);

        if(!item->loadFromJSON(cableObj))
        {
            delete item;
            delete cable;
            continue;
        }

        addCable(item);
    }

    const QJsonArray nodes = obj.value("nodes").toArray();

    for(const QJsonValue& v : nodes)
    {
        const QJsonObject nodeObj = v.toObject();
        const QString nodeType = nodeObj.value("type").toString();
        if(nodeType.isEmpty())
            continue;

        // Create new node
        AbstractNodeGraphItem *item = factory->createItem(nodeType, this);
        if(item)
        {
            if(!item->loadFromJSON(nodeObj))
            {
                auto *node = item->getAbstractNode();
                delete item;
                delete node;
                continue;
            }

            addNode(item);
        }
    }

    // Recalculate circuits
    calculateConnections();

    setHasUnsavedChanges(false);

    return true;
}

void CircuitScene::saveToJSON(QJsonObject &obj) const
{
    obj["name"] = circuitSheetName();
    obj["long_name"] = circuitSheetLongName();

    QJsonArray cables;
    for(const auto& it : mCables)
    {
        CableGraphItem *item = it.second;

        QJsonObject cableObj;
        item->saveToJSON(cableObj);
        cables.append(cableObj);
    }

    obj["cables"] = cables;

    QJsonArray nodes;
    for(const auto& it : mItemMap)
    {
        AbstractNodeGraphItem *item = it.second;
        QJsonObject nodeObj;
        item->saveToJSON(nodeObj);
        nodes.append(nodeObj);
    }

    obj["nodes"] = nodes;
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
    modeMgr()->setEditingSubMode(EditingSubMode::CableEditing);

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
    modeMgr()->setEditingSubMode(EditingSubMode::CableEditing);

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

    // Reset editing sub mode
    modeMgr()->setEditingSubMode(EditingSubMode::Default);

    if(!apply)
        return;

    setHasUnsavedChanges(true);

    if(!cablePath.isComplete())
        return; // TODO: error message

    if(isNew)
    {
        // Create new cable
        CircuitCable *cable = new CircuitCable(circuitsModel()->modeMgr(), this);
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
    bool consumed = true;

    switch (e->key())
    {
    case Qt::Key_Escape:
        modeMgr()->setEditingSubMode(EditingSubMode::Default);
        break;
    case Qt::Key_S:
        if(e->modifiers() == Qt::NoModifier && modeMgr()->mode() == FileMode::Editing)
            modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if(isEditingCable())
            endEditCable(true);
        break;
    default:
        consumed = false;
        break;
    }

    if(!consumed &&
            modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
    {
        consumed = true;

        if(e->matches(QKeySequence::Copy))
        {
            copySelectedItems();
        }
        else if(e->matches(QKeySequence::Cut))
        {
            copySelectedItems();
            removeSelectedItems();
        }
        else if(e->keyCombination() == QKeyCombination(Qt::ControlModifier, Qt::Key_I))
        {
            invertSelection();
        }
        else if(e->keyCombination() == QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_A))
        {
            clearSelection();
            mSelectedItemPositions.clear();
            mSelectedCablePositions.clear();
        }
        else
        {
            consumed = false;
        }

        // TODO: clear selection and invertselection
    }

    if(!consumed &&
            modeMgr()->mode() == FileMode::Editing)
    {
        consumed = true;

        if(e->matches(QKeySequence::SelectAll))
        {
            // Force start selection
            modeMgr()->setEditingSubMode(EditingSubMode::ItemSelection);
            selectAll();
        }
        else
        {
            consumed = false;
        }
    }

    if(!consumed && isEditingCable() && e->matches(QKeySequence::Undo))
    {
        consumed = true;

        editCableUndoLast();
    }

    if(consumed)
    {
        e->accept();
        return;
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

void CircuitScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    const QRectF sr = sceneRect();

    // Actual scene background
    QRectF bgRect = rect.intersected(sr);

    if(!sr.contains(rect))
    {
        // Dark gray backgound outside of scene rect
        painter->fillRect(rect, Qt::darkGray);
    }

    // Scene white background
    painter->fillRect(bgRect, Qt::white);

    if(circuitsModel()->modeMgr()->mode() == FileMode::Editing)
    {
        // Draw grid lines in gray
        QPen gridPen(Qt::gray, 2.0);
        painter->setPen(gridPen);

        const int ARR_SIZE = 100;
        QLineF arr[ARR_SIZE];

        const qreal x1 = bgRect.left();
        const qreal x2 = bgRect.right();
        const qreal t  = bgRect.top();
        const qreal b  = bgRect.bottom();

        const int minH = qCeil(sr.top() / TileLocation::Size);
        const int maxH  = qFloor(sr.bottom() / TileLocation::Size);
        const int minV = qCeil(sr.left() / TileLocation::Size);
        const int maxV  = qFloor(sr.right() / TileLocation::Size);

        if(minH > maxH || minV > maxV)
            return; // Nothing to draw

        int firstH = qCeil(t / TileLocation::Size) - 1;
        int lastH  = qFloor(b / TileLocation::Size) + 1;
        int firstV = qCeil(x1 / TileLocation::Size) - 1;
        int lastV  = qFloor(x2 / TileLocation::Size) + 1;

        firstH = qBound(minH, firstH, maxH);
        lastH = qBound(minH, lastH, maxH);
        firstV = qBound(minV, firstV, maxV);
        lastV = qBound(minV, lastV, maxV);

        const int horizN = lastH - firstH;
        if (horizN > 0)
        {
            qreal y = firstH * TileLocation::Size;
            int count = 0;
            while(count < horizN)
            {
                int i = 0;
                for (; i < ARR_SIZE; i++)
                {
                    arr[i] = QLineF(x1, y, x2, y);
                    y += TileLocation::Size;

                    if(count >= horizN)
                        break;
                    count++;
                }
                painter->drawLines(arr, i + 1);
            }
        }

        const int vertN = lastV - firstV;
        if (vertN > 0)
        {
            qreal x = firstV * TileLocation::Size;
            int count = 0;
            while(count < vertN)
            {
                int i = 0;
                for (; i < ARR_SIZE; i++)
                {
                    arr[i] = QLineF(x, t, x, b);
                    x += TileLocation::Size;

                    if(count >= vertN)
                        break;
                    count++;
                }
                painter->drawLines(arr, i + 1);
            }
        }
    }
}

CircuitListModel *CircuitScene::circuitsModel() const
{
    return static_cast<CircuitListModel *>(parent());
}
