/**
 * src/circuits/graphs/cablegraphitem.h
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

#ifndef CABLEGRAPHITEM_H
#define CABLEGRAPHITEM_H

#include <QGraphicsObject>
#include <QPainterPath>
#include <QPen>
#include <QVector>

#include "../../utils/tilerotate.h"

class CircuitCable;
class CircuitScene;

class QJsonObject;

class CableGraphPath
{
public:
    CableGraphPath() = default;

    inline const QVector<TileLocation>& tiles() const
    {
        return mTiles;
    }

    inline int getTilesCount() const
    {
        return mTiles.size();
    }

    inline bool isEmpty() const
    {
        return mTiles.isEmpty();
    }

    inline TileLocation first() const { return mTiles.first(); }
    inline TileLocation last() const { return mTiles.last(); }
    inline TileLocation at(int i) const { return mTiles.at(i); }

    Connector::Direction getEnterDirection(int tileIdx) const;
    Connector::Direction getExitDirection(int tileIdx) const;

    bool addTile(const TileLocation& l);

    Connector::Direction startDirection() const;
    bool setStartDirection(Connector::Direction newStartDirection);

    Connector::Direction endDirection() const;
    bool setEndDirection(Connector::Direction newEndDirection);

    bool removeLastLine();

    bool isPointInsideCableTiles(const QPointF& pos) const;

    QPainterPath generatePath() const;

    inline bool isZeroLength() const
    {
        return mIsZeroLength;
    }

    inline bool isComplete() const
    {
        return mPathIsComplete;
    }

    CableGraphPath tryMerge(const CableGraphPath& other) const;

    typedef std::pair<CableGraphPath, CableGraphPath> SplitPair;
    bool splitted(const TileLocation splitLoc, SplitPair& result) const;

    CableGraphPath translatedBy(int16_t dx, int16_t dy) const;

    static CableGraphPath createZeroLength(const TileLocation& a,
                                           const TileLocation& b);

    static CableGraphPath loadFromJSON(const QJsonObject& obj);
    static void saveToJSON(const CableGraphPath& path, QJsonObject& obj);

private:
    static Connector::Direction getDirection(const TileLocation& a,
                                             const TileLocation& b);

private:
    QVector<TileLocation> mTiles;
    Connector::Direction mStartDirection;
    Connector::Direction mEndDirection;

    bool mExitDirectionIsFree = true;
    Connector::Direction mWantedExitDirection = Connector::Direction::South;

    bool mPathIsComplete = false;
    bool mIsZeroLength = false;
};

class CableGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    CableGraphItem(CircuitCable *cable_);

    CircuitScene *circuitScene() const;

    QPainterPath path() const;

    const CableGraphPath& cablePath() const;

    void setCablePath(const CableGraphPath &newCablePath)
    {
        setCablePathInternal(newCablePath, true);
    }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    CircuitCable *cable() const;

    bool cableZeroLength() const;

    TileLocation sideA() const;

    TileLocation sideB() const;

    Connector::Direction directionA() const;

    Connector::Direction directionB() const;

    bool loadFromJSON(const QJsonObject& obj);
    void saveToJSON(QJsonObject& obj) const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    friend class CircuitScene;
    void setPathInternal(const QPainterPath& newPath);

    void setCablePathInternal(const CableGraphPath &newCablePath,
                              bool registerTiles);

    bool isMouseInsideShapePluseExtra(const QPointF& p) const;

private slots:
    void updatePen();
    void triggerUpdate();

private:
    CircuitCable *mCable;
    QPen pen;
    QPainterPath mPath;
    QRectF mBoundingRect;
    CableGraphPath mCablePath;
};

#endif // CABLEGRAPHITEM_H
