/**
 * src/circuits/graphs/abstractnodegraphitem.cpp
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

#include "abstractnodegraphitem.h"

#include "../nodes/abstractcircuitnode.h"

#include "../circuitscene.h"

#include <QPainter>
#include <QFont>

#include <QGraphicsSceneMouseEvent>

#include <QJsonObject>

AbstractNodeGraphItem::AbstractNodeGraphItem(AbstractCircuitNode *node_)
    : QGraphicsObject()
    , mAbstractNode(node_)
{
    setParent(mAbstractNode);

    connect(mAbstractNode, &QObject::objectNameChanged,
            this, &AbstractNodeGraphItem::updateName);
    connect(mAbstractNode, &AbstractCircuitNode::circuitsChanged,
            this, &AbstractNodeGraphItem::triggerUpdate);
    connect(mAbstractNode, &AbstractCircuitNode::shapeChanged,
            this, &AbstractNodeGraphItem::onShapeChanged);

    updateName();

    setFlag(ItemSendsGeometryChanges, true);
}

QRectF AbstractNodeGraphItem::boundingRect() const
{
    return QRectF(0, 0, TileLocation::Size, TileLocation::Size);
}

void AbstractNodeGraphItem::triggerUpdate()
{
    update();
}

void AbstractNodeGraphItem::updateName()
{
    setToolTip(mAbstractNode->objectName());
    update();
}

void AbstractNodeGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->mode() == FileMode::Editing && !s->isEditingCable() && boundingRect().contains(ev->pos()))
    {
        // Sometimes we receive clicks even if out of node tile
        // In those cases do not start moving item or rotate it!
        if(ev->button() == Qt::LeftButton)
        {
            s->startMovingItem(this);
        }
        else if(ev->button() == Qt::RightButton)
        {
            // Rotate counter/clockwise 90 (Shift)
            TileRotate delta = TileRotate::Deg90;
            if(ev->modifiers() & Qt::ShiftModifier)
                delta = TileRotate::Deg270; // Opposite direction

            setRotate(rotate() + delta);
        }
    }

    QGraphicsObject::mousePressEvent(ev);
}

void AbstractNodeGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->mode() == FileMode::Editing)
    {
        // After move has ended we go back to last valid location
        s->endMovingItem();
    }

    QGraphicsObject::mouseReleaseEvent(ev);
}

void AbstractNodeGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->mode() == FileMode::Editing)
    {
        // Prevent accidental move while editing item
        s->endMovingItem();

        if(ev->button() == Qt::LeftButton)
        {
            s->requestEditNode(this);
            return;
        }
    }

    QGraphicsObject::mouseDoubleClickEvent(ev);
}

QVariant AbstractNodeGraphItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == GraphicsItemChange::ItemPositionChange)
    {
        // Snap to grid
        QPointF newPos = value.toPointF();
        newPos.rx() = std::round(newPos.x() / TileLocation::Size) * TileLocation::Size;
        newPos.ry() = std::round(newPos.y() / TileLocation::Size) * TileLocation::Size;

        if(newPos != pos() && scene())
        {
            CircuitScene *s = circuitScene();
            TileLocation newLocation = TileLocation::fromPoint(newPos);
            if(!s->updateItemLocation(newLocation, this))
            {
                // New position was not free
                // Reset to old position
                return pos();
            }
        }
        return newPos;
    }
    else if(change == GraphicsItemChange::ItemPositionHasChanged)
    {
        // Detach all contacts, will be revaluated later
        invalidateConnections(false);
    }

    return QGraphicsObject::itemChange(change, value);
}

void AbstractNodeGraphItem::drawMorsetti(QPainter *painter, int nodeContact, TileRotate r)
{
    Q_ASSERT(nodeContact >= 0 && nodeContact < getAbstractNode()->getContactCount());

    QLineF morsettoLine;

    QRectF morsettoEllipse;
    morsettoEllipse.setSize(QSizeF(13.0, 13.0));

    QRectF textRect1;
    textRect1.setSize(QSizeF(40.0, 22.0));

    QRectF textRect2 = textRect1;

    Qt::Alignment text1Align = Qt::AlignVCenter;
    Qt::Alignment text2Align = Qt::AlignVCenter;

    switch (toConnectorDirection(r))
    {
    case Connector::Direction::North:
        morsettoLine.setP1(QPointF(TileLocation::HalfSize,  0.0));
        morsettoLine.setP2(QPointF(TileLocation::HalfSize, 22.0));
        morsettoEllipse.moveCenter(QPointF(TileLocation::HalfSize, 11.0));

        // Text 1 is on the left
        textRect1.moveTopLeft(QPointF(1.0, 0.0));
        textRect2.moveTopRight(QPointF(TileLocation::Size - 1.0, 0.0));

        text1Align |= Qt::AlignRight;
        text2Align |= Qt::AlignLeft;
        break;

    case Connector::Direction::South:
        morsettoLine.setP1(QPointF(TileLocation::HalfSize,
                                   TileLocation::Size));
        morsettoLine.setP2(QPointF(TileLocation::HalfSize,
                                   TileLocation::Size - 22.0));
        morsettoEllipse.moveCenter(QPointF(TileLocation::HalfSize,
                                           TileLocation::Size - 11.0));

        // Text 1 still on the left
        textRect1.moveBottomLeft(QPointF(1.0,
                                         TileLocation::Size));
        textRect2.moveBottomRight(QPointF(TileLocation::Size - 1.0,
                                          TileLocation::Size));

        text1Align |= Qt::AlignRight;
        text2Align |= Qt::AlignLeft;
        break;

    case Connector::Direction::East:
        morsettoLine.setP1(QPointF(TileLocation::Size - 22.0,
                                   TileLocation::HalfSize));
        morsettoLine.setP2(QPointF(TileLocation::Size,
                                   TileLocation::HalfSize));
        morsettoEllipse.moveCenter(QPointF(TileLocation::Size - 11.0,
                                           TileLocation::HalfSize));

        // Text 1 goes above Text 2
        textRect1.moveBottomRight(QPointF(TileLocation::Size - 1.0,
                                          TileLocation::HalfSize - 6.0));
        textRect2.moveTopRight(QPointF(TileLocation::Size - 1.0,
                                       TileLocation::HalfSize + 6.0));

        text1Align |= Qt::AlignRight;
        text2Align |= Qt::AlignRight;
        break;

    case Connector::Direction::West:
        morsettoLine.setP1(QPointF(0.0,
                                   TileLocation::HalfSize));
        morsettoLine.setP2(QPointF(22.0,
                                   TileLocation::HalfSize));
        morsettoEllipse.moveCenter(QPointF(11.0,
                                           TileLocation::HalfSize));

        // Text 1 still goes above Text 2
        textRect1.moveBottomLeft(QPointF(1.0,
                                         TileLocation::HalfSize - 6.0));
        textRect2.moveTopLeft(QPointF(1.0,
                                      TileLocation::HalfSize + 6.0));

        text1Align |= Qt::AlignLeft;
        text2Align |= Qt::AlignLeft;
        break;
    default:
        break;
    }

    QColor color = Qt::black;
    if(getAbstractNode()->hasCircuit(nodeContact,
                                     CircuitType::Closed))
        color = Qt::red;
    else if(getAbstractNode()->hasCircuit(nodeContact,
                                     CircuitType::Open))
        color.setRgb(120, 210, 255); // Light blue

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(5.0);
    pen.setColor(color);

    painter->setPen(pen);
    painter->drawLine(morsettoLine);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(morsettoEllipse);

    QFont f;
    f.setPointSizeF(12.0);
    f.setItalic(true);
    painter->setFont(f);

    const auto& contact = getAbstractNode()->getContacts().at(nodeContact);

    painter->setPen(Qt::black);
    painter->drawText(textRect1, contact.name1, text1Align);

    painter->drawText(textRect2, contact.name2, text2Align);
}

void AbstractNodeGraphItem::drawName(QPainter *painter,
                                     const QString& name,
                                     TileRotate r,
                                     QRectF *br)
{
    QRectF textRect;

    Qt::Alignment textAlign = Qt::AlignVCenter;

    switch (toConnectorDirection(r - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        textRect.setLeft(26.0);
        textRect.setWidth(50.0);
        textRect.setBottom(TileLocation::HalfSize - 12.0);
        textRect.setTop(5.0);

        textAlign |= Qt::AlignLeft;
        break;

    case Connector::Direction::South:
        textRect.setLeft(26.0);
        textRect.setWidth(50.0);
        textRect.setTop(TileLocation::HalfSize + 12.0);
        textRect.setBottom(TileLocation::Size - 5.0);

        textAlign |= Qt::AlignLeft;
        break;

    case Connector::Direction::East:
        textRect.setLeft(TileLocation::HalfSize + 3.0);
        textRect.setRight(TileLocation::Size - 5.0);
        textRect.setTop(23.0);
        textRect.setBottom(TileLocation::HalfSize);

        textAlign |= Qt::AlignRight;
        break;

    case Connector::Direction::West:
        textRect.setLeft(3.0);
        textRect.setRight(TileLocation::HalfSize - 5.0);
        textRect.setTop(23.0);
        textRect.setBottom(TileLocation::HalfSize);

        textAlign |= Qt::AlignLeft;
        break;
    default:
        break;
    }

    QFont f;
    f.setPointSizeF(18.0);
    f.setBold(true);

    QFontMetrics metrics(f, painter->device());
    double width = metrics.horizontalAdvance(name, QTextOption(textAlign));
    if(width > textRect.width())
    {
        f.setPointSizeF(f.pointSizeF() * textRect.width() / width * 0.9);
    }

    painter->setFont(f);
    painter->drawText(textRect, textAlign, name, br);

    if(br)
    {
        // Cut bounding rect exceeding text rect
        *br = br->intersected(textRect);
    }
}

void AbstractNodeGraphItem::invalidateConnections(bool tryReconnectImmediately)
{
    // Disable all contacts. Will be re-evaluated when move ends
    // Or when Editing mode finishes
    // Or immediately if requested
    CircuitScene *s = circuitScene();
    if(s)
        s->refreshItemConnections(this, tryReconnectImmediately);
}

TileRotate AbstractNodeGraphItem::rotate() const
{
    return mRotate;
}

void AbstractNodeGraphItem::setRotate(TileRotate newRotate)
{
    if(mRotate == newRotate)
        return;
    mRotate = newRotate;

    // Detach all contacts, try reconnect immediately
    invalidateConnections();

    CircuitScene *s = circuitScene();
    if(s)
        s->setHasUnsavedChanges(true);

    update();
}

CircuitScene *AbstractNodeGraphItem::circuitScene() const
{
    return qobject_cast<CircuitScene *>(scene());
}

bool AbstractNodeGraphItem::loadFromJSON(const QJsonObject &obj)
{
    if(!getAbstractNode()->loadFromJSON(obj))
        return false;

    TileLocation tile{0, 0};
    tile.x = obj.value("x").toInt();
    tile.y = obj.value("y").toInt();
    setLocation(tile);

    setRotate(TileRotate(obj.value("rotation").toInt()));

    return true;
}

void AbstractNodeGraphItem::saveToJSON(QJsonObject &obj) const
{
    TileLocation tile = location();
    obj["x"] = tile.x;
    obj["y"] = tile.y;

    obj["rotation"] = int(mRotate);

    getAbstractNode()->saveToJSON(obj);
}

void AbstractNodeGraphItem::onShapeChanged()
{
    // Detach all contacts, will be revaluated immediately
    invalidateConnections();
    update();
}
