/**
 * src/circuits/graphs/cablegraphitem.cpp
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

#include "cablegraphitem.h"

#include "../nodes/circuitcable.h"
#include "../nodes/abstractcircuitnode.h"

#include "../circuitscene.h"
#include "../../views/modemanager.h"

#include <QPainterPathStroker>
#include <QPainter>

#include <QJsonObject>
#include <QJsonArray>

#include <QGraphicsSceneMouseEvent>

static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);
    if (path == QPainterPath() || pen == Qt::NoPen)
        return path;
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    if (pen.widthF() <= 0.0)
        ps.setWidth(penWidthZero);
    else
        ps.setWidth(pen.widthF());
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}

CableGraphItem::CableGraphItem(CircuitCable *cable_)
    : QGraphicsObject()
    , mCable(cable_)
{
    setParent(mCable);

    pen.setColor(Qt::black);
    pen.setWidthF(5.0);

    connect(mCable, &CircuitCable::modeChanged, this, &CableGraphItem::updatePen);
    connect(mCable, &CircuitCable::powerChanged, this, &CableGraphItem::updatePen);
    connect(mCable, &CircuitCable::nodesChanged, this, &CableGraphItem::triggerUpdate);

    updatePen();
}

QRectF CableGraphItem::boundingRect() const
{
    if (mBoundingRect.isNull())
    {
        CableGraphItem *self = const_cast<CableGraphItem*>(this);
        qreal pw = pen.style() == Qt::NoPen ? qreal(0) : pen.widthF();
        if (pw == 0.0)
            self->mBoundingRect = mPath.controlPointRect();
        else
            self->mBoundingRect = shape().controlPointRect();
    }
    return mBoundingRect;
}

QPainterPath CableGraphItem::shape() const
{
    return qt_graphicsItem_shapeFromPath(mPath, pen);
}

void CableGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    auto *s = circuitScene();

    bool isAconnected = mCable->getNode(CableSide::A).node;
    bool isBconnected = mCable->getNode(CableSide::B).node;

    bool showUnconnected = !isAconnected && !isBconnected;
    if(s && s->mode() == FileMode::Editing)
    {
        // In editing mode even if just one side is not connected
        // still draw attention to this cable
        showUnconnected = !isAconnected || !isBconnected;
    }

    QColor oldColor = pen.color();
    if(s && s->mode() == FileMode::Editing && isSelected())
    {
        pen.setColor(Qt::darkCyan);
    }
    else if(showUnconnected)
    {
        // Draw line with light orange
        pen.setColor(qRgb(255, 178, 102));
    }

    // Draw cable path
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(mPath);

    pen.setColor(oldColor);
}

void CableGraphItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
    {
        if(ev->buttons() & Qt::LeftButton)
        {
            s->moveSelectedCableAt(TileLocation::fromPointFloor(ev->scenePos()));

            // Eat the event, we bypass normal item move logic
            return;
        }
    }

    QGraphicsObject::mouseMoveEvent(ev);
}

void CableGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->modeMgr()->editingSubMode() == EditingSubMode::ItemSelection)
    {
        if(!ev->buttons().testFlag(Qt::LeftButton))
        {
            s->endSelectionMove();
            // Do not eat event to let QGraphicsItem process select/unselect
        }
    }

    QGraphicsObject::mouseReleaseEvent(ev);
}

void CableGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && ev->button() == Qt::LeftButton)
    {
        s->requestEditCable(this);
        return;
    }

    QGraphicsObject::mouseDoubleClickEvent(ev);
}

QVariant CableGraphItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    CircuitScene *s = circuitScene();
    switch (change)
    {
    case GraphicsItemChange::ItemSelectedHasChanged:
    {
        s->onCableSelected(this, isSelected());
        break;
    }
    default:
        break;
    }

    return QGraphicsObject::itemChange(change, value);
}

void CableGraphItem::setPathInternal(const QPainterPath &newPath)
{
    prepareGeometryChange();
    mPath = newPath;
    mBoundingRect = QRectF();
    update();
}

CircuitScene *CableGraphItem::circuitScene() const
{
    return qobject_cast<CircuitScene *>(scene());
}

QPainterPath CableGraphItem::path() const
{
    return mPath;
}

const CableGraphPath &CableGraphItem::cablePath() const
{
    return mCablePath;
}

void CableGraphItem::setCablePath(const CableGraphPath &newCablePath)
{
    prepareGeometryChange();

    // Update scene
    auto *s = circuitScene();
    if(s)
        s->removeCableTiles(this);

    mCablePath = newCablePath;

    if(s)
        s->addCableTiles(this);

    mPath = mCablePath.generatePath();
    mBoundingRect = QRectF();

    setVisible(!mCablePath.isZeroLength());

    // Detach nodes, they will be reattached later
    CableEnd cableEnd = cable()->getNode(CableSide::A);
    if(cableEnd.node)
    {
        CableItem item;
        item.cable.cable = cable();
        item.cable.side = CableSide::A;
        item.nodeContact = cableEnd.nodeContact;

        item.cable.pole = CircuitPole::First;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }

        item.cable.pole = CircuitPole::Second;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }
    }

    cableEnd = cable()->getNode(CableSide::B);
    if(cableEnd.node)
    {
        CableItem item;
        item.cable.cable = cable();
        item.cable.side = CableSide::B;
        item.nodeContact = cableEnd.nodeContact;

        item.cable.pole = CircuitPole::First;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }

        item.cable.pole = CircuitPole::Second;
        if(cableEnd.node->getContactType(cableEnd.nodeContact, item.cable.pole) != ContactType::NotConnected)
        {
            cableEnd.node->detachCable(item);
        }
    }

    update();
}

void CableGraphItem::updatePen()
{
    prepareGeometryChange();
    mBoundingRect = QRectF();

    const auto power = mCable->powered();
    const auto powerPole = toCablePowerPole(power);
    const auto powerType = toCircuitType(power);

    QColor color = Qt::black;
    if(powerPole != CablePowerPole::None)
    {
        if(powerType == CircuitType::Closed)
            color = Qt::red;
        else
            color.setRgb(120, 210, 255); // Light blue
    }

    Qt::PenStyle style = Qt::SolidLine;
    if(powerPole != CablePowerPole::None &&
            powerPole != CablePowerPole::Both &&
            mCable->mode() == CircuitCable::Mode::Unifilar)
        style = Qt::DashLine;

    pen.setColor(color);
    pen.setStyle(style);

    if(powerPole == CablePowerPole::None)
        setZValue(0);
    else if(powerType == CircuitType::Open)
        setZValue(1);
    else
        setZValue(2); // Closed circuits on top

    update();
}

void CableGraphItem::triggerUpdate()
{
    update();
}

Connector::Direction CableGraphItem::directionA() const
{
    return mCablePath.startDirection();
}

Connector::Direction CableGraphItem::directionB() const
{
    return mCablePath.endDirection();
}

bool CableGraphItem::loadFromJSON(const QJsonObject &obj)
{
    if(!obj.contains("path"))
        return false;

    CableGraphPath newPath;

    const QJsonObject pathObj = obj.value("path").toObject();

    const QJsonArray tiles = pathObj.value("tiles").toArray();

    bool zero = pathObj.value("zero").toBool(false);
    if(zero)
    {
        if(tiles.size() != 2)
            return false;

        QJsonObject tileObj = tiles.first().toObject();
        TileLocation a;
        a.x = int16_t(tileObj.value("x").toInt());
        a.y = int16_t(tileObj.value("y").toInt());

        tileObj = tiles.last().toObject();
        TileLocation b;
        b.x = int16_t(tileObj.value("x").toInt());
        b.y = int16_t(tileObj.value("y").toInt());

        newPath = CableGraphPath::createZeroLength(a, b);
    }
    else
    {
        int startDir = pathObj.value("start_dir").toInt();
        int endDir = pathObj.value("end_dir").toInt();

        newPath.setStartDirection(Connector::Direction(startDir));

        for(const QJsonValue& v : tiles)
        {
            const QJsonObject tileObj = v.toObject();
            TileLocation tile;
            tile.x = int16_t(tileObj.value("x").toInt());
            tile.y = int16_t(tileObj.value("y").toInt());
            newPath.addTile(tile);
        }

        newPath.setEndDirection(Connector::Direction(endDir));

        if(!newPath.isComplete())
            return false;
    }

    setCablePath(newPath);
    return true;
}

void CableGraphItem::saveToJSON(QJsonObject &obj) const
{
    QJsonObject pathObj;
    pathObj["zero"] = mCablePath.isZeroLength();

    QJsonArray tiles;
    for(const TileLocation tile : mCablePath.tiles())
    {
        QJsonObject tileObj;
        tileObj["x"] = tile.x;
        tileObj["y"] = tile.y;
        tiles.append(tileObj);
    }
    pathObj["tiles"] = tiles;

    if(!mCablePath.isZeroLength())
    {
        pathObj["start_dir"] = int(mCablePath.startDirection());
        pathObj["end_dir"] = int(mCablePath.endDirection());
    }

    obj["path"] = pathObj;
}

TileLocation CableGraphItem::sideA() const
{
    if(mCablePath.isEmpty())
        return TileLocation::invalid;

    if(mCablePath.isZeroLength())
        return mCablePath.first();

    return mCablePath.first() + mCablePath.startDirection();
}

TileLocation CableGraphItem::sideB() const
{
    if(mCablePath.isEmpty())
        return TileLocation::invalid;

    if(mCablePath.isZeroLength())
        return mCablePath.last();

    return mCablePath.last() + mCablePath.endDirection();
}

bool CableGraphItem::cableZeroLength() const
{
    return mCablePath.isZeroLength();
}

CircuitCable *CableGraphItem::cable() const
{
    return mCable;
}

Connector::Direction CableGraphPath::getEnterDirection(int tileIdx) const
{
    if(tileIdx == 0)
        return mStartDirection;

    if(tileIdx >= mTiles.size())
        return Connector::Direction::South; // Error

    const TileLocation& tile = mTiles.at(tileIdx);
    const TileLocation& prevTile = mTiles.at(tileIdx - 1);
    return getDirection(tile, prevTile);
}

Connector::Direction CableGraphPath::getExitDirection(int tileIdx) const
{
    if(tileIdx == mTiles.size() - 1)
        return mEndDirection;

    if(tileIdx >= mTiles.size())
        return Connector::Direction::South; // Error

    const TileLocation& tile = mTiles.at(tileIdx);
    const TileLocation& nextTile = mTiles.at(tileIdx + 1);
    return getDirection(tile, nextTile);
}

bool CableGraphPath::addTile(const TileLocation &l)
{
    if(mPathIsComplete)
        return false;

    if(!mTiles.isEmpty())
    {
        if(mTiles.last() == l)
            return false; // Cannot add same tile twice

        const TileLocation& prevTile = mTiles.last();
        if(prevTile.x != l.x && prevTile.y != l.y)
        {
            // Adjacent tiles must have one coordinate in common
            return false;
        }

        const auto newPrevTileExitDir = getDirection(mTiles.last(), l);
        const auto newTileStartDir = ~newPrevTileExitDir; // Opposite
        if(!mExitDirectionIsFree)
        {
            // We didn't respect last exit direction
            if(newPrevTileExitDir != mWantedExitDirection)
                return false;
        }

        // Check we are not going back
        const auto prevTileStartDir = getEnterDirection(mTiles.size() - 1);
        if(newPrevTileExitDir == prevTileStartDir)
            return false;

        // Cable can pass on same tile twice if:
        // - it's not same as previous tile
        // - Both times it passes it does not bend, but goes straight
        // - Direction is different in the 2 passages

        int idx = mTiles.indexOf(l);
        if(idx >= 0)
        {
            // Check if first passage was straigh
            const auto enterDir1 = getEnterDirection(idx);
            const auto exitDir1 = getExitDirection(idx);
            if(enterDir1 != ~exitDir1)
                return false; // Directions are not opposite, it bends

            if(newTileStartDir == enterDir1 || newTileStartDir == exitDir1)
                return false; // Both passages have same direction

            // We do not want to bend second passage
            mExitDirectionIsFree = false;
            mWantedExitDirection = ~newTileStartDir; // Opposite
        }
        else
        {
            // Reset
            mExitDirectionIsFree = true;
        }
    }

    mTiles.append(l);
    return true;
}

Connector::Direction CableGraphPath::startDirection() const
{
    return mStartDirection;
}

bool CableGraphPath::setStartDirection(Connector::Direction newStartDirection)
{
    if(!mTiles.isEmpty())
        return false; // Must be set before adding tiles

    mStartDirection = newStartDirection;
    return true;
}

Connector::Direction CableGraphPath::endDirection() const
{
    return mEndDirection;
}

bool CableGraphPath::setEndDirection(Connector::Direction newEndDirection)
{
    if(mTiles.isEmpty())
        return false; // Must add tiles first

    if(!mExitDirectionIsFree && newEndDirection != mWantedExitDirection)
    {
        // We didn't respect last exit direction
        return false;
    }

    if(getEnterDirection(mTiles.size() - 1) == newEndDirection)
        return false; // Cannot enter and exit on same direction

    mEndDirection = newEndDirection;

    mPathIsComplete = true;

    return true;
}

bool CableGraphPath::removeLastLine()
{
    if(mTiles.isEmpty() || isZeroLength())
        return false;

    if(mPathIsComplete)
    {
        // Remove end connector direction
        mPathIsComplete = false;
        return true;
    }

    if(mTiles.size() == 1)
    {
        // Remove first
        mTiles.removeLast();
        return true;
    }

    // Remove last straight line
    const auto startDir = getEnterDirection(mTiles.size() - 1);
    while(mTiles.size() > 1) // Do not remove first
    {
        const auto otherStartDir = getEnterDirection(mTiles.size() - 1);
        if(otherStartDir != startDir)
            break; // We reached a bend

        mTiles.removeLast();
    }

    return true;
}

QPainterPath CableGraphPath::generatePath() const
{
    QPainterPath path;
    if(mTiles.isEmpty() || isZeroLength())
        return path; // Empty

    // Move to start connector
    path.moveTo(CircuitScene::getConnectorPoint(mTiles.first(), mStartDirection));

    // Draw start connector line
    QPointF center = mTiles.first().toPoint();
    center.rx() += TileLocation::HalfSize;
    center.ry() += TileLocation::HalfSize;
    path.lineTo(center);

    Connector::Direction exitDir = getExitDirection(0);
    for(int i = 1; i < mTiles.size() - 1; i++)
    {
        // Go from second to just before last tile
        Connector::Direction exitDir2 = getExitDirection(i);
        if(exitDir2 == exitDir)
            continue; // Line is straigh, go to next tile

        // We reached a bend
        center = mTiles.at(i).toPoint();
        center.rx() += TileLocation::HalfSize;
        center.ry() += TileLocation::HalfSize;
        path.lineTo(center);

        exitDir = exitDir2;
    }

    if(mTiles.size() > 1)
    {
        // Go to last tile
        center = mTiles.last().toPoint();
        center.rx() += TileLocation::HalfSize;
        center.ry() += TileLocation::HalfSize;
        path.lineTo(center);
    }

    if(mPathIsComplete)
    {
        // Draw end connector line
        path.lineTo(CircuitScene::getConnectorPoint(mTiles.last(), mEndDirection));
    }

    return path;
}

CableGraphPath CableGraphPath::tryMerge(const CableGraphPath &other) const
{
    // No point in merging zero length cables
    if(isZeroLength() || other.isZeroLength())
        return {}; // Return empty

    CableGraphPath totalPath;

    TileLocation ourNextFirst = this->first() + this->startDirection();
    if(ourNextFirst == other.first()
            && other.startDirection() == ~this->startDirection())
    {
        // Pass other cable reversed and then ourselves
        totalPath.setStartDirection(other.endDirection());
        for(int i = other.getTilesCount() - 1; i >= 0; i--)
        {
            if(!totalPath.addTile(other.at(i)))
            {
                return {};
            }
        }

        for(int i = 0; i < this->getTilesCount(); i++)
        {
            if(!totalPath.addTile(this->at(i)))
            {
                return {};
            }
        }

        if(!totalPath.setEndDirection(this->endDirection()))
            return {};

        return totalPath;
    }

    if(ourNextFirst == other.last()
            && other.endDirection() == ~this->startDirection())
    {
        // Pass other cable and then ourselves
        totalPath.setStartDirection(other.startDirection());
        for(int i = 0; i < other.getTilesCount(); i++)
        {
            if(!totalPath.addTile(other.at(i)))
            {
                return {};
            }
        }

        for(int i = 0; i < this->getTilesCount(); i++)
        {
            if(!totalPath.addTile(this->at(i)))
            {
                return {};
            }
        }

        if(!totalPath.setEndDirection(this->endDirection()))
            return {};

        return totalPath;
    }

    TileLocation ourNextLast = this->last() + this->endDirection();
    if(ourNextLast == other.first()
            && other.startDirection() == ~this->endDirection())
    {
        // Pass ourselves and then other cable
        totalPath.setStartDirection(this->startDirection());

        for(int i = 0; i < this->getTilesCount(); i++)
        {
            if(!totalPath.addTile(this->at(i)))
            {
                return {};
            }
        }

        for(int i = 0; i < other.getTilesCount(); i++)
        {
            if(!totalPath.addTile(other.at(i)))
            {
                return {};
            }
        }

        if(!totalPath.setEndDirection(other.endDirection()))
            return {};

        return totalPath;
    }

    if(ourNextLast == other.last()
            && other.endDirection() == ~this->endDirection())
    {
        // Pass ourselves and then other cable reversed
        totalPath.setStartDirection(this->startDirection());

        for(int i = 0; i < this->getTilesCount(); i++)
        {
            if(!totalPath.addTile(this->at(i)))
            {
                return {};
            }
        }

        for(int i = other.getTilesCount() - 1; i >= 0; i--)
        {
            if(!totalPath.addTile(other.at(i)))
            {
                return {};
            }
        }

        if(!totalPath.setEndDirection(other.startDirection()))
            return {};

        return totalPath;
    }

    // Return empty
    return {};
}

bool CableGraphPath::splitted(const TileLocation splitLoc, SplitPair& result) const
{
    if(isZeroLength() || !mTiles.contains(splitLoc))
        return false; // Nothing to split

    CableGraphPath a, b;

    // Set a start
    a.setStartDirection(this->startDirection());

    int tileIdx = 0;
    for(; tileIdx < mTiles.size(); tileIdx++)
    {
        const TileLocation& tile = mTiles.at(tileIdx);

        if(tile == splitLoc)
        {
            // We reached node tile
            if(a.isEmpty())
                break; // No path before tile

            // Set end direction
            a.setEndDirection(getDirection(a.last(), splitLoc));
            break;
        }

        // Copy first half to a
        a.addTile(tile);
    }

    // Skip splitLoc tile
    tileIdx++;

    for(; tileIdx < mTiles.size(); tileIdx++)
    {
        const TileLocation& tile = mTiles.at(tileIdx);

        if(b.isEmpty())
        {
            // Set b start direction
            b.setStartDirection(getDirection(tile, splitLoc));
        }

        // Copy second half to b
        b.addTile(tile);
    }

    b.setEndDirection(this->endDirection());

    result.first = a;
    result.second = b;
    return true;
}

CableGraphPath CableGraphPath::translatedBy(int16_t dx, int16_t dy) const
{
    if(isZeroLength() || isEmpty())
        return {};

    CableGraphPath translated;
    translated.setStartDirection(startDirection());

    for(const TileLocation tile : mTiles)
    {
        translated.addTile(tile.adjusted(dx, dy));
    }

    translated.setEndDirection(endDirection());

    return translated;
}

CableGraphPath CableGraphPath::createZeroLength(const TileLocation &a, const TileLocation &b)
{
    CableGraphPath path;
    path.mIsZeroLength = true;

    if(a.x == b.x)
    {
        Q_ASSERT(std::abs(a.y - b.y) == 1);
        path.mStartDirection = getDirection(b, a); // Opposite
    }
    else if(a.y == b.y)
    {
        Q_ASSERT(std::abs(a.x - b.x) == 1);
        path.mStartDirection = getDirection(b, a); // Opposite
    }
    else
    {
        // Tiles are not adjacent
        Q_ASSERT(false);
    }

    path.mTiles.append(a);
    path.mTiles.append(b);
    path.mEndDirection = ~path.mStartDirection;

    return path;
}

Connector::Direction CableGraphPath::getDirection(const TileLocation &a, const TileLocation &b)
{
    if(a.x == b.x)
    {
        if(a.y > b.y)
            return Connector::Direction::North;
        return Connector::Direction::South;
    }

    // Tiles have same y
    if(a.x > b.x)
        return Connector::Direction::West;
    return Connector::Direction::East;
}
