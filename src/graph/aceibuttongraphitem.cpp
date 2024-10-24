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
    constexpr double arcRadius = 15.0;

    constexpr QLineF centerToNorth(center.x(), center.y() - arcRadius,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + arcRadius,
                                   center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast(center.x() + arcRadius, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - arcRadius, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;
    QLineF contact1Line;
    QLineF contact2Line;

    const bool contact1On = node()->state() == ACEIButtonNode::State::Pressed;
    const bool contact2On = node()->state() == ACEIButtonNode::State::Normal || node()->state() == ACEIButtonNode::State::Pressed;

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

    // Now draw wires on top
    const QColor colors[3] =
    {
        QColor(255, 140, 140), // Light red, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    QLineF lines[3] =
    {
        commonLine,
        contact1Line,
        contact2Line
    };

    // Fill edges
    pen.setCapStyle(Qt::SquareCap);

    // Draw all circuits with polyline to fill the edges
    // Start from turned off contacts, then open and then closed circuits
    AnyCircuitType targetType = AnyCircuitType::None;
    bool finishedDrawingContacts = false;
    while(!finishedDrawingContacts)
    {
        // Set pen color based on circuit type
        pen.setColor(colors[int(targetType)]);
        painter->setPen(pen);

        bool passThrough = false;

        for(int contact = 0; contact < 3; contact++)
        {
            const AnyCircuitType state = node()->hasAnyCircuit(contact);

            if(contact == 0 && targetType == AnyCircuitType::Open
                    && node()->hasAnyCircuit(0) != AnyCircuitType::None)
            {
                // Common always reaches opposite side arc.
                // We already draw it for closed and none circuts.
                // But for open circuit we draw it on both sides
                // until before arc.
                for(int other = contact + 1; other < 3; other++)
                {
                    QPointF points[3] =
                    {
                        lines[contact].p2(),
                        center,
                        lines[other].p1() // Before arc
                    };
                    painter->drawPolyline(points, 3);
                }
            }

            // Always draw full line if in None state
            if(state != targetType && targetType != AnyCircuitType::None)
                continue;

            // Just draw line from after arc
            painter->drawLine(lines[contact]);

            for(int other = contact + 1; other < 3; other++)
            {
                // Draw full circuit, passing center
                // Always draw full line if in None state
                AnyCircuitType otherState = node()->hasAnyCircuit(other);
                if(otherState != targetType && targetType != AnyCircuitType::None)
                    continue;

                if(contact != 0 && (!contact1On || !contact2On))
                    continue; // There is no circuit between 1 and 2 contacts

                passThrough = true;

                QPointF points[3] =
                {
                    lines[contact].p2(),
                    center,
                    lines[other].p2(),
                };
                painter->drawPolyline(points, 3);
            }
        }

        if(targetType == AnyCircuitType::Open)
        {
            // Draw Arc over open circuits
            // But below closed circuits

            const QRectF arcRect(center.x() - arcRadius,
                                 center.y() - arcRadius,
                                 arcRadius * 2, arcRadius * 2);

            if(node()->hasCircuits(CircuitType::Closed))
                passThrough = true; // Closed circuits always pass through

            if((contact1On || contact2On) && passThrough)
            {
                // Switch is on (at least one side)
                // Set correct pen if has also closed circuits
                if(node()->hasCircuits(CircuitType::Closed))
                    pen.setColor(colors[int(AnyCircuitType::Closed)]);
            }
            else
            {
                // Switch is off
                pen.setColor(colors[int(AnyCircuitType::None)]);
            }

            painter->setPen(pen);
            painter->drawArc(arcRect,
                             startAngle * 16,
                             (endAngle - startAngle) * 16);
        }

        // Go to next state
        switch (targetType)
        {
        case AnyCircuitType::None:
            targetType = AnyCircuitType::Open;
            break;
        case AnyCircuitType::Open:
            targetType = AnyCircuitType::Closed;
            break;
        case AnyCircuitType::Closed:
        default:
            finishedDrawingContacts = true;
            break;
        }
    }

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