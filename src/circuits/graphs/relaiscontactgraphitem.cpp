/**
 * src/circuits/graphs/relaiscontactgraphitem.cpp
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

#include "relaiscontactgraphitem.h"

#include "../nodes/relaiscontactnode.h"
#include "../../objects/relais/model/abstractrelais.h"

#include <QPainter>

RelaisContactGraphItem::RelaisContactGraphItem(RelaisContactNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RelaisContactNode::stateChanged,
            this, &RelaisContactGraphItem::triggerUpdate);
}

void RelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

    bool contact1On = node()->state() == RelaisContactNode::State::Up;
    bool contact2On = node()->state() == RelaisContactNode::State::Down;
    if(node()->swapContactState())
        std::swap(contact1On, contact2On);

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
    const int arcLength = endAngle - startAngle;

    TileRotate centralConnectorRotate = TileRotate::Deg90;
    if(node()->flipContact())
        centralConnectorRotate = TileRotate::Deg270;

    drawMorsetti(painter, 0, rotate() + TileRotate::Deg0);
    drawMorsetti(painter, 2, rotate() + TileRotate::Deg180);
    if(node()->hasCentralConnector())
        drawMorsetti(painter, 1, rotate() + centralConnectorRotate);

    // Draw switch arc and wires on top
    const QColor colors[3] =
    {
        QColor(120, 210, 255), // Light blue, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    QLineF lines[3] =
    {
        commonLine,
        contact1Line,
        contact2Line
    };

    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(5.0);

    // Fill edges with miter join
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    pen.setColor(colors[int(AnyCircuitType::None)]);
    painter->setPen(pen);

    // Draw full switch arc below wires
    // On relay contact, the 2 contacts cannot be
    // turned on at same time, so arc is always black
    const QRectF arcRect(center.x() - arcRadius,
                         center.y() - arcRadius,
                         arcRadius * 2, arcRadius * 2);

    painter->drawArc(arcRect,
                     startAngle * 16,
                     arcLength * 16);

    // Draw all circuits with polyline to fill the edges
    // Start from turned off contacts, then open and then closed circuits
    AnyCircuitType targetType = AnyCircuitType::None;
    bool finishedDrawingContacts = false;
    while(!finishedDrawingContacts)
    {
        // Set pen color based on circuit type
        pen.setColor(colors[int(targetType)]);
        painter->setPen(pen);

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

            bool passThrough = false;

            for(int other = 0; other < 3; other++)
            {
                if(other == contact)
                    continue;

                // Draw full circuit, passing center
                // Always draw full line if in None state
                if(targetType != AnyCircuitType::None)
                {
                    AnyCircuitType otherState = node()->hasAnyCircuit(other);
                    if(otherState != targetType)
                        continue;

                    // If both sides have voltage, check if it's really a circuit
                    // (switch is on) or if the 2 voltages come from external sources
                    // (in this case switch is off)
                    if(!contact1On && (other == 1 || contact == 1))
                        continue;

                    if(!contact2On && (other == 2 || contact == 2))
                        continue;
                }

                passThrough = true;

                QPointF points[3] =
                {
                    lines[contact].p2(),
                    center,
                    lines[other].p2(),
                };
                painter->drawPolyline(points, 3);
            }

            if(!passThrough)
            {
                // Just draw line from after arc
                painter->drawLine(lines[contact]);
            }
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

    // If not turned on, redraw partial black arc over wire
    pen.setColor(colors[int(AnyCircuitType::None)]);
    painter->setPen(pen);

    int lateralArcStart = startAngle;
    int topArcStart = (endAngle - arcLength / 2);
    if(node()->flipContact())
        std::swap(topArcStart, lateralArcStart);

    if(!contact1On)
    {
        // Draw lateral half of arc
        painter->drawArc(arcRect,
                         lateralArcStart * 16,
                         (arcLength / 2) * 16);
    }

    if(!contact2On)
    {
        // Draw top half of arc
        painter->drawArc(arcRect,
                         topArcStart * 16,
                         (arcLength / 2) * 16);
    }

    // Draw name
    if(node()->relais())
    {
        QColor color = Qt::black;

        switch (node()->relais()->state())
        {
        case AbstractRelais::State::Up:
            color = Qt::red;
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color.setRgb(120, 210, 255); // Light blue
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }

        painter->setPen(color);

        TileRotate nameRotate = rotate();
        if(node()->flipContact())
            nameRotate += TileRotate::Deg180;
        drawName(painter, node()->objectName(), nameRotate);
    }
}

void RelaisContactGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    TileRotate centralConnectorRotate = TileRotate::Deg90;
    if(node()->flipContact())
        centralConnectorRotate = TileRotate::Deg270;

    connectors.emplace_back(location(), rotate(), 0); // Common
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 2); // Down
    if(node()->hasCentralConnector())
        connectors.emplace_back(location(), rotate() + centralConnectorRotate, 1);  // Up
}

RelaisContactNode *RelaisContactGraphItem::node() const
{
    return static_cast<RelaisContactNode *>(getAbstractNode());
}
