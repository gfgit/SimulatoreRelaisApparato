/**
 * src/circuits/graphs/levercontactgraphitem.cpp
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

#include "levercontactgraphitem.h"

#include "../nodes/levercontactnode.h"
#include "../../objects/lever/model/genericleverobject.h"

#include "../../views/modemanager.h"

#include <QPainter>

LeverContactGraphItem::LeverContactGraphItem(LeverContactNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &LeverContactNode::stateChanged,
            this, &LeverContactGraphItem::triggerUpdate);
}

void LeverContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // TODO: very similar to RelaisContactNode paint, make common

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

    LeverContactNode::State state = node()->state();

    if(node()->lever() &&
            node()->modeMgr()->mode() != FileMode::Simulation)
    {
        // In static or editing mode,
        // draw contact with lever in its normal state
        const int leverNormalPos = node()->lever()->positionDesc().normalPositionIdx;

        state = LeverContactNode::State::Up;
        if(node()->isPositionOn(leverNormalPos))
            state = LeverContactNode::State::Down;
    }

    bool contact1On = state == LeverContactNode::State::Up;
    bool contact2On = state == LeverContactNode::State::Down;
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

    TileRotate nameRotate = rotate();
    if(node()->flipContact())
        nameRotate += TileRotate::Deg180;

    // Draw name and lever conditions
    if(node()->lever())
    {
        // Draw lever conditions
        drawLeverConditions(painter, nameRotate);

        // Draw lever name
        QColor color = Qt::black;
        painter->setPen(color);
        drawName(painter, node()->objectName(), nameRotate);
    }
    else
    {
        // Draw lever name
        QColor color = Qt::red;
        painter->setPen(color);
        drawName(painter, tr("Null"), nameRotate);
    }
}

void LeverContactGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    TileRotate centralConnectorRotate = TileRotate::Deg90;
    if(node()->flipContact())
        centralConnectorRotate = TileRotate::Deg270;

    connectors.emplace_back(location(), rotate(), 0); // Common
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 2); // Down
    if(node()->hasCentralConnector())
        connectors.emplace_back(location(), rotate() + centralConnectorRotate, 1);  // Up
}

LeverContactNode *LeverContactGraphItem::node() const
{
    return static_cast<LeverContactNode *>(getAbstractNode());
}

void LeverContactGraphItem::drawLeverConditions(QPainter *painter, TileRotate r)
{
    // Draw lever conditions
    // Positioning is similar to text but opposite side
    QRectF conditionsRect;

    switch (toConnectorDirection(r - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        // We go south, right/left (flipped)
        if(node()->flipContact())
        {
            conditionsRect.setLeft(15.0);
            conditionsRect.setRight(TileLocation::HalfSize - 3.5);
        }
        else
        {
            conditionsRect.setLeft(TileLocation::HalfSize + 3.5);
            conditionsRect.setRight(TileLocation::Size - 15.0);
        }
        conditionsRect.setTop(TileLocation::HalfSize + 10.0);
        conditionsRect.setBottom(TileLocation::Size - 15.0);
        break;

    case Connector::Direction::South:
        // We go north left/right (flipped)
        if(node()->flipContact())
        {
            conditionsRect.setLeft(TileLocation::HalfSize + 3.5);
            conditionsRect.setRight(TileLocation::Size - 15.0);
        }
        else
        {
            conditionsRect.setLeft(15.0);
            conditionsRect.setRight(TileLocation::HalfSize - 3.5);
        }
        conditionsRect.setBottom(TileLocation::HalfSize - 10.0);
        conditionsRect.setTop(15.0);
        break;

    case Connector::Direction::East:
        conditionsRect.setLeft(TileLocation::HalfSize + 3.0);
        conditionsRect.setRight(TileLocation::Size - 5.0);
        conditionsRect.setTop(TileLocation::HalfSize);
        conditionsRect.setBottom(TileLocation::Size - 23.0);
        break;

    case Connector::Direction::West:
        conditionsRect.setLeft(3.0);
        conditionsRect.setRight(TileLocation::HalfSize - 5.0);
        conditionsRect.setTop(TileLocation::HalfSize);
        conditionsRect.setBottom(TileLocation::Size - 23.0);
        break;
    default:
        break;
    }

    const QPointF leverCenter = conditionsRect.center();

    constexpr double circleRadius = 4;
    constexpr double leverLength = 12;

    QRectF circle;
    circle.setSize(QSizeF(circleRadius * 2,
                          circleRadius * 2));
    circle.moveCenter(leverCenter);

    QPen pen;
    pen.setCapStyle(Qt::SquareCap);
    pen.setColor(Qt::black);
    pen.setWidthF(2.5);

    // Draw condition lines
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QPointF endPt;
    const auto conditions = node()->conditionSet();

    const auto& positionDesc = node()->lever()->positionDesc();

    QRectF arcRect(QPointF(),
                   QSizeF(leverLength * 2, leverLength * 2));
    arcRect.moveCenter(leverCenter);

    for(const LeverPositionCondition& item : conditions)
    {
        const double fromAngle = positionDesc.previewAngleFor(item.positionFrom);

        // Zero is vertical up, so cos/sin are swapped
        // Also returned angle must be inverted to be clockwise
        const double fromRadiants = -qDegreesToRadians(fromAngle);

        endPt = QPointF(qSin(fromRadiants), qCos(fromRadiants));
        endPt *= -leverLength; // Negative to go upwards
        endPt += leverCenter;

        painter->drawLine(leverCenter, endPt);

        if(item.type == LeverPositionConditionType::FromTo)
        {
            // Draw end position line and
            // Draw arc from start position to end position
            const double toAngle = positionDesc.previewAngleFor(item.positionTo);
            const double toRadiants = -qDegreesToRadians(toAngle);

            endPt = QPointF(qSin(toRadiants), qCos(toRadiants));
            endPt *= -leverLength; // Negative to go upwards
            endPt += leverCenter;

            painter->drawLine(leverCenter, endPt);

            // drawArc wants degrees multiplied by 16
            // Counter-clockwise and starting from 3 o'clock
            // so +90 and inverted sign
            painter->drawArc(arcRect, (90 - fromAngle) * 16,
                             (fromAngle - toAngle) * 16);
        }
    }

    QColor leverColor = Qt::darkCyan;
    if(node()->state() == LeverContactNode::Down)
        leverColor = Qt::red;
    pen.setColor(leverColor);

    const bool drawState = node()->modeMgr()->mode() == FileMode::Simulation;
    if(drawState)
    {
        // Draw current lever state only in Simulation

        // Draw lever line
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        double leverAngle = 0;

        const int leverPos = node()->lever()->position();
        if(positionDesc.isMiddle(leverPos))
        {
            // Average prev/next angles
            const double prev = positionDesc.previewAngleFor(leverPos - 1);
            const double next = positionDesc.previewAngleFor(leverPos + 1);
            leverAngle = (prev + next) / 2.0;
        }
        else
        {
            leverAngle = positionDesc.previewAngleFor(leverPos);
        }

        const double leverAngleRadiants = -qDegreesToRadians(leverAngle);

        endPt = QPointF(qSin(leverAngleRadiants), qCos(leverAngleRadiants));
        endPt *= -leverLength; // Negative to go upwards
        endPt += leverCenter;
        painter->drawLine(leverCenter, endPt);
    }

    // Draw circle
    painter->setBrush(drawState ? leverColor : Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle);
}
