/**
 * src/circuits/graphs/abstractnodegraphitem.h
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

#ifndef ABSTRACTNODEGRAPHITEM_H
#define ABSTRACTNODEGRAPHITEM_H

#include <QGraphicsObject>

#include "../../utils/tilerotate.h"

class AbstractCircuitNode;
class CircuitScene;

class QJsonObject;

class AbstractNodeGraphItem : public QGraphicsObject
{
    Q_OBJECT
public:
    AbstractNodeGraphItem(AbstractCircuitNode *node_);

    QRectF boundingRect() const override;

    virtual void getConnectors(std::vector<Connector>& /*connectors*/) const {}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    inline AbstractCircuitNode *getAbstractNode() const
    {
        return mAbstractNode;
    }

    inline TileLocation location() const
    {
        return TileLocation::fromPoint(pos());
    }

    inline void setLocation(const TileLocation& l)
    {
        setPos(l.toPoint());
    }

    TileRotate rotate() const;
    void setRotate(TileRotate newRotate);

    CircuitScene *circuitScene() const;

    virtual bool loadFromJSON(const QJsonObject& obj);
    virtual void saveToJSON(QJsonObject& obj) const;

protected slots:
    void triggerUpdate();
    virtual void updateName();
    void onShapeChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void drawMorsetti(QPainter *painter, int nodeContact, TileRotate r);
    void drawName(QPainter *painter,
                  const QString &name,
                  TileRotate r,
                  QRectF *br = nullptr);

    void invalidateConnections(bool tryReconnectImmediately = true);

private:
    AbstractCircuitNode *mAbstractNode;
    TileRotate mRotate = TileRotate::Deg0;
};

#endif // ABSTRACTNODEGRAPHITEM_H
