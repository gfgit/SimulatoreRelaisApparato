/**
 * src/circuits/graphs/abstractdeviatorgraphitem.cpp
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

#include "abstractdeviatorgraphitem.h"

#include "../nodes/abstractdeviatornode.h"

#include "circuitcolors.h"

#include <QPainter>

Connector::Direction AbstractDeviatorGraphItem::calculateArcSide() const
{
    Connector::Direction arcSide = Connector::Direction::North;

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        if(deviatorNode()->flipContact())
            arcSide = Connector::Direction::West;
        else
            arcSide = Connector::Direction::East;
        break;

    case Connector::Direction::South:
        if(deviatorNode()->flipContact())
            arcSide = Connector::Direction::East;
        else
            arcSide = Connector::Direction::West;
        break;

    case Connector::Direction::East:
        if(deviatorNode()->flipContact())
            arcSide = Connector::Direction::North;
        else
            arcSide = Connector::Direction::South;
        break;

    case Connector::Direction::West:
        if(deviatorNode()->flipContact())
            arcSide = Connector::Direction::South;
        else
            arcSide = Connector::Direction::North;
        break;
    default:
        break;
    }

    return arcSide;
}

AbstractDeviatorGraphItem::AbstractDeviatorGraphItem(AbstractDeviatorNode *node_)
    : AbstractNodeGraphItem{node_}
{
    connect(deviatorNode(), &AbstractDeviatorNode::deviatorStateChanged,
            this, &AbstractDeviatorGraphItem::triggerUpdate);
}

void AbstractDeviatorGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    TileRotate centralConnectorRotate = TileRotate::Deg90;
    if(deviatorNode()->flipContact())
        centralConnectorRotate = TileRotate::Deg270;

    connectors.emplace_back(location(), rotate(), 0); // Common
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 2); // Down
    if(deviatorNode()->hasCentralConnector())
        connectors.emplace_back(location(), rotate() + centralConnectorRotate, 1);  // Up
}

double AbstractDeviatorGraphItem::textDisplayFontSize() const
{
    return 20.0; // pt, a bit smaller than relay power nodes
}

QRectF AbstractDeviatorGraphItem::textDisplayRect() const
{
    const Connector::Direction arcSide = calculateArcSide();

    const double textDisplayHeight = textDisplayFontSize() * 1.2;
    QRectF textRect;
    switch (textRotate())
    {
    case Connector::Direction::North:
        textRect.setTop(- TextDisplayMarginSmall - textDisplayHeight);
        textRect.setBottom(0);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);

        if(arcSide != textRotate())
            textRect.moveTop(textRect.top() + TileLocation::HalfSize / 2.0);
        break;
    case Connector::Direction::South:
        textRect.setTop(TileLocation::Size);
        textRect.setBottom(TileLocation::Size + TextDisplayMarginSmall + textDisplayHeight);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);

        if(arcSide != textRotate())
            textRect.moveTop(textRect.top() - TileLocation::HalfSize / 2.0);
        break;
    case Connector::Direction::East:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::Size);
        textRect.setLeft(TileLocation::Size);
        textRect.setRight(TileLocation::Size + TextDisplayMarginSmall + mTextWidth);

        if(arcSide != textRotate())
            textRect.moveLeft(textRect.left() - TileLocation::HalfSize / 2.0);

        if(deviatorNode()->hasCentralConnector())
            textRect.moveLeft(textRect.left() + 2);
        break;
    case Connector::Direction::West:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::Size);
        textRect.setLeft(- TextDisplayMarginSmall - mTextWidth);
        textRect.setRight(0);

        if(arcSide != textRotate())
            textRect.moveLeft(textRect.left() + TileLocation::HalfSize / 2.0);

        if(deviatorNode()->hasCentralConnector())
            textRect.moveLeft(textRect.left() - 2);
        break;
    default:
        break;
    }

    return textRect;
}

AbstractDeviatorNode *AbstractDeviatorGraphItem::deviatorNode() const
{
    return static_cast<AbstractDeviatorNode *>(getAbstractNode());
}

const QString AbstractDeviatorGraphItem::getContactTooltip() const
{
    bool contact1On = deviatorNode()->isContactOn(AbstractDeviatorNode::DownIdx);
    bool contact2On = deviatorNode()->isContactOn(AbstractDeviatorNode::UpIdx);

    const QString onStr = tr("On");
    const QString offStr = tr("Off");

    if(deviatorNode()->hasCentralConnector())
    {
        return tr("Contacts:<br>"
                  "Straight: <b>%1</b><br>"
                  "Central:  <b>%2</b>")
                .arg(contact1On ? onStr : offStr,
                     contact2On ? onStr : offStr);
    }
    else
    {
        return tr("Contact: <b>%1</b>")
                .arg(contact1On ? onStr : offStr);
    }
}

void AbstractDeviatorGraphItem::drawDeviator(QPainter *painter, bool contactUpOn, bool contactDownOn)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    constexpr QLineF centerToNorth(center.x(), center.y() - arcRadius,
                                   center.x(), 0);

    constexpr QLineF centerToSouth(center.x(), center.y() + arcRadius,
                                   center.x(), TileLocation::Size);

    constexpr QLineF centerToEast(center.x() + arcRadius, center.y(),
                                  TileLocation::Size, center.y());

    constexpr QLineF centerToWest(center.x() - arcRadius, center.y(),
                                  0, center.y());

    // Same but ends slightly before tile rect border
    constexpr QLineF centerToNorth2(center.x(), center.y() - arcRadius,
                                    center.x(), morsettiOffset);

    constexpr QLineF centerToSouth2(center.x(), center.y() + arcRadius,
                                    center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast2(center.x() + arcRadius, center.y(),
                                   TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest2(center.x() - arcRadius, center.y(),
                                   morsettiOffset, center.y());

    QLineF commonLine;
    QLineF contact1Line;
    QLineF contact2Line;

    int startAngle = 0;
    int endAngle = 0;

    const bool hasCentral = deviatorNode()->hasCentralConnector();

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        if(deviatorNode()->flipContact())
            contact1Line = hasCentral ? centerToWest : centerToWest2;
        else
            contact1Line = hasCentral ? centerToEast : centerToEast2;
        contact2Line = centerToSouth;

        startAngle = 0;
        endAngle = -90;
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        if(deviatorNode()->flipContact())
            contact1Line = hasCentral ? centerToEast : centerToEast2;
        else
            contact1Line = hasCentral ? centerToWest : centerToWest2;
        contact2Line = centerToNorth;

        startAngle = -180;
        endAngle = -270;
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        if(deviatorNode()->flipContact())
            contact1Line = hasCentral ? centerToNorth : centerToNorth2;
        else
            contact1Line = hasCentral ? centerToSouth : centerToSouth2;
        contact2Line = centerToWest;

        startAngle = -90;
        endAngle = -180;
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        if(deviatorNode()->flipContact())
            contact1Line = hasCentral ? centerToSouth : centerToSouth2;
        else
            contact1Line = hasCentral ? centerToNorth : centerToNorth2;
        contact2Line = centerToEast;

        startAngle = 90;
        endAngle = 0;
        break;
    default:
        break;
    }

    int angleIncrement = 35;
    if(deviatorNode()->flipContact())
    {
        startAngle -= 90;
        endAngle -= 90;
        angleIncrement = -angleIncrement;
    }

    int startIncrement = 0;
    int endIncrement = 0;

    if(!contactUpOn)
        startIncrement += angleIncrement;
    if(!contactDownOn)
        endIncrement -= angleIncrement;

    if(deviatorNode()->flipContact())
        std::swap(startIncrement, endIncrement);

    startAngle += startIncrement;
    endAngle += endIncrement;
    const int arcLength = endAngle - startAngle;

    // TileRotate centralConnectorRotate = TileRotate::Deg90;
    // if(deviatorNode()->flipContact())
    //     centralConnectorRotate = TileRotate::Deg270;

    // drawMorsetti(painter, 0, rotate() + TileRotate::Deg0);
    // drawMorsetti(painter, 2, rotate() + TileRotate::Deg180);
    // if(deviatorNode()->hasCentralConnector())
    //     drawMorsetti(painter, 1, rotate() + centralConnectorRotate);

    // Draw switch arc and wires on top
    const QColor colors[3] =
    {
        CircuitColors::Open,
        CircuitColors::Closed,
        CircuitColors::None
    };

    QLineF lines[3] =
    {
        commonLine,
        contact1Line,
        contact2Line
    };

    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);

    // Fill edges with miter join
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    // Draw black custom arc below everything
    pen.setColor(colors[int(AnyCircuitType::None)]);
    painter->setPen(pen);

    if(deviatorNode()->hasCentralConnector() && deviatorNode()->bothCanBeActive())
    {
        // When both contacts can be active at same time we draw
        // a special diagonal line inside arc

        // Draw black middle diagonal line below everything
        QPointF corner;
        corner.setX(contact1Line.x2());
        corner.setY(contact1Line.y2());
        if(qFuzzyCompare(contact1Line.x2(), center.x()))
            corner.setX(contact2Line.x2());
        if(qFuzzyCompare(contact1Line.y2(), center.y()))
            corner.setY(contact2Line.y2());

        // Going to corner would make diagonal longer
        // than central contact line (sqrt(2) * length)
        // So we set direction and then adjust length to be same
        QLineF diagonal(center, corner);
        diagonal.setLength(qAbs(center.x() - corner.x()));
        painter->drawLine(diagonal);
    }

    // Draw full switch arc below wires
    const QRectF arcRect(center.x() - arcRadius,
                         center.y() - arcRadius,
                         arcRadius * 2, arcRadius * 2);

    // Default to black arc
    if((contactUpOn && contactDownOn))
    {
        // Both sides are turned on, draw color on switch arc
        // Set correct pen if has also closed circuits
        if(deviatorNode()->hasCircuits(CircuitType::Closed))
            pen.setColor(colors[int(AnyCircuitType::Closed)]);
        else if(deviatorNode()->hasCircuits(CircuitType::Open))
            pen.setColor(colors[int(AnyCircuitType::Open)]);
    }

    painter->setPen(pen);
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
            const AnyCircuitType state = deviatorNode()->hasAnyCircuit(contact);

            if(contact == 0 && targetType == AnyCircuitType::Open
                    && deviatorNode()->hasAnyCircuit(0) != AnyCircuitType::None)
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
                    AnyCircuitType otherState = deviatorNode()->hasAnyCircuit(other);
                    if(otherState != targetType)
                        continue;

                    // If both sides have voltage, check if it's really a circuit
                    // (switch is on) or if the 2 voltages come from external sources
                    // (in this case switch is off)
                    if(!contactUpOn && (other == 1 || contact == 1))
                        continue;

                    if(!contactDownOn && (other == 2 || contact == 2))
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
    if(deviatorNode()->flipContact())
        std::swap(topArcStart, lateralArcStart);

    if(!contactUpOn)
    {
        // Draw lateral half of arc
        painter->drawArc(arcRect,
                         lateralArcStart * 16,
                         (arcLength / 2) * 16);
    }

    if(!contactDownOn)
    {
        // Draw top half of arc
        painter->drawArc(arcRect,
                         topArcStart * 16,
                         (arcLength / 2) * 16);
    }
}
