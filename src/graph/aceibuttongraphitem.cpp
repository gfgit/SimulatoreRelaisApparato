/**
 * src/graph/aceibuttongraphitem.cpp
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

#include "aceibuttongraphitem.h"

#include "../nodes/aceibuttonnode.h"

#include "circuitscene.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

ACEIButtonGraphItem::ACEIButtonGraphItem(ACEIButtonNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &ACEIButtonNode::stateChanged,
            this, &ACEIButtonGraphItem::triggerUpdate);
    connect(node(), &ACEIButtonNode::shapeChanged,
            this, &ACEIButtonGraphItem::onShapeChanged);
}

void ACEIButtonGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;

    constexpr QLineF centerToNorth(center,
                               QPointF(center.x(), morsettiOffset));

    constexpr QLineF centerToSouth(center,
                               QPointF(center.x(),
                                       TileLocation::Size - morsettiOffset));

    constexpr QLineF centerToEast(center,
                              QPointF(TileLocation::Size - morsettiOffset,
                                      center.y()));

    constexpr QLineF centerToWest(center,
                              QPointF(morsettiOffset,
                                      center.y()));

    QLineF commonLine;
    QLineF contact1Line;
    QLineF contact2Line;

    bool contact1On = node()->state() == ACEIButtonNode::State::Pressed;
    bool contact2On = node()->state() == ACEIButtonNode::State::Normal || node()->state() == ACEIButtonNode::State::Pressed;

    int startAngle = 0;
    int endAngle = 0;

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        if(node()->flipContact())
            contact1Line = centerToWest;
        else
            contact1Line = centerToEast;
        contact2Line = centerToSouth;

        startAngle = 0;
        endAngle = -90;
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        if(node()->flipContact())
            contact1Line = centerToEast;
        else
            contact1Line = centerToWest;
        contact2Line = centerToNorth;

        startAngle = -180;
        endAngle = -270;
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        if(node()->flipContact())
            contact1Line = centerToNorth;
        else
            contact1Line = centerToSouth;
        contact2Line = centerToWest;

        startAngle = -90;
        endAngle = -180;
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        if(node()->flipContact())
            contact1Line = centerToSouth;
        else
            contact1Line = centerToNorth;
        contact2Line = centerToEast;

        startAngle = 90;
        endAngle = 0;
        break;
    default:
        break;
    }

    int angleIncrement = 35;
    if(node()->flipContact())
    {
        startAngle -= 90;
        endAngle -= 90;
        angleIncrement = -angleIncrement;
    }

    int startIncrement = 0;
    int endIncrement = 0;

    if(!contact1On)
        startIncrement += angleIncrement;
    if(!contact2On)
        endIncrement -= angleIncrement;

    if(node()->flipContact())
        std::swap(startIncrement, endIncrement);

    startAngle += startIncrement;
    endAngle += endIncrement;

    TileRotate centralConnectorRotate = TileRotate::Deg90;
    if(node()->flipContact())
        centralConnectorRotate = TileRotate::Deg270;

    drawMorsetti(painter, 0, rotate() + TileRotate::Deg0);
    drawMorsetti(painter, 1, rotate() + centralConnectorRotate);
    drawMorsetti(painter, 2, rotate() + TileRotate::Deg180);

    // Draw wires
    QPen pen;
    pen.setWidthF(5.0);
    pen.setColor(Qt::black);
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawLine(commonLine);
    painter->drawLine(contact1Line);
    painter->drawLine(contact2Line);

    // Draw middle diagonal line
    QPointF corner;
    corner.setX(contact1Line.x2());
    corner.setY(contact1Line.y2());
    if(qFuzzyCompare(contact1Line.x2(), center.x()))
        corner.setX(contact2Line.x2());
    if(qFuzzyCompare(contact1Line.y2(), center.y()))
        corner.setY(contact2Line.y2());
    painter->drawLine(center, corner);

    // Draw Arc
    const QRectF arcRect(center.x() - 15,
                         center.y() - 15,
                         30, 30);

    painter->drawArc(arcRect,
                     startAngle * 16,
                     (endAngle - startAngle) * 16);

    // Now draw powered wires on top
    pen.setColor(Qt::red);
    painter->setPen(pen);

    if(node()->hasCircuit(0))
        painter->drawLine(commonLine);
    if(node()->hasCircuit(1))
        painter->drawLine(contact1Line);
    if(node()->hasCircuit(2))
        painter->drawLine(contact2Line);

    // Draw name
    QColor color = Qt::black;

    switch (node()->state())
    {
    case ACEIButtonNode::State::Pressed:
        color = Qt::darkGreen;
        break;
    case ACEIButtonNode::State::Extracted:
        color = Qt::red;
        break;
    case ACEIButtonNode::State::Normal:
    default:
        break;
    }

    painter->setPen(color);

    TileRotate nameRotate = rotate();
    if(node()->flipContact())
        nameRotate += TileRotate::Deg180;
    drawName(painter, node()->objectName(), nameRotate);
}

void ACEIButtonGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    TileRotate centralConnectorRotate = TileRotate::Deg90;
    if(node()->flipContact())
        centralConnectorRotate = TileRotate::Deg270;

    connectors.emplace_back(location(), rotate(), 0); // Common
    connectors.emplace_back(location(), rotate() + centralConnectorRotate, 1);  // Up
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 2); // Down
}

ACEIButtonNode *ACEIButtonGraphItem::node() const
{
    return static_cast<ACEIButtonNode *>(getAbstractNode());
}

void ACEIButtonGraphItem::onShapeChanged()
{
    // Detach all contacts, will be revaluated later
    invalidateConnections();
    update();
}

void ACEIButtonGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->mode() == CircuitScene::Mode::Simulation)
    {
        if(ev->button() == Qt::LeftButton)
            node()->setState(ACEIButtonNode::State::Pressed);
        else if(ev->button() == Qt::RightButton)
            node()->setState(ACEIButtonNode::State::Extracted);
        return;
    }

    AbstractNodeGraphItem::mousePressEvent(ev);
}

void ACEIButtonGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->mode() == CircuitScene::Mode::Simulation)
    {
        if(ev->button() == Qt::LeftButton || ev->button() == Qt::RightButton)
            node()->setState(ACEIButtonNode::State::Normal);
        return;
    }

    AbstractNodeGraphItem::mouseReleaseEvent(ev);
}
