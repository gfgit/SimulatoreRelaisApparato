/**
 * src/circuits/graphs/simplenodegraphitem.cpp
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

#include "simplenodegraphitem.h"

#include "../nodes/simplecircuitnode.h"

#include <QPainter>

SimpleNodeGraphItem::SimpleNodeGraphItem(SimpleCircuitNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void SimpleNodeGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    // We do not draw morsetti on this node

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double centerOffset = 22.0;

    QLineF lines[4] =
    {
        // Center to Deg0 South
        {center.x(), center.y() + centerOffset,
         center.x(), TileLocation::Size},

        // Center to Deg90 West
        {center.x() - centerOffset, center.y(),
         0, center.y()},

        // Center to Deg180 Nord
        {center.x(), center.y() - centerOffset,
         center.x(), 0},

        // Center to Deg270 East
        {center.x() + centerOffset, center.y(),
         TileLocation::Size, center.y()}
    };


    // Now draw wires on top
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);

    // Fill edges with miter join
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    const QColor colors[3] =
    {
        QColor(120, 210, 255), // Light blue, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    const int startIdx = toRotateInt(rotate());
    const QLineF common = lines[startIdx];

    // Draw all circuits with polyline to fill the edges
    // Start from turned off contacts, then open and then closed circuits
    AnyCircuitType targetType = AnyCircuitType::None;
    bool finishedDrawingContacts = false;
    while(!finishedDrawingContacts)
    {
        // Set pen color based on circuit type
        pen.setColor(colors[int(targetType)]);
        painter->setPen(pen);

        for(int contact = 0; contact < 4; contact++)
        {
            if(contact != 0 && node()->disabledContact() == contact)
                continue; // Common contact is never disabled

            const AnyCircuitType state = node()->hasAnyCircuit(contact);

            // Always draw full line if in None state
            if(state != targetType && targetType != AnyCircuitType::None)
                continue;

            const QLineF circuit = lines[(startIdx + contact) % 4];

            bool passThrough = false;
            for(int other = 0; other < 4; other++)
            {
                if(other == contact)
                    continue;

                if(node()->disabledContact() == other)
                    continue;

                // Draw full circuit, passing center
                // Always draw full line if in None state
                AnyCircuitType otherState = node()->hasAnyCircuit(other);
                if(otherState != targetType && targetType != AnyCircuitType::None)
                    continue;

                passThrough = true;

                const QLineF otherCircuit = lines[(startIdx + other) % 4];

                QPointF points[5] =
                {
                    otherCircuit.p2(),
                    otherCircuit.p1(),
                    common.p1(),
                    circuit.p1(),
                    circuit.p2()
                };
                painter->drawPolyline(points, 5);
            }

            if(!passThrough)
            {
                // Draw only until center
                if(contact == 0)
                {
                    painter->drawLine(common);
                }
                else
                {
                    QPointF points[3] =
                    {
                        circuit.p2(),
                        circuit.p1(),
                        common.p1()
                    };
                    painter->drawPolyline(points, 3);
                }
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
}

void SimpleNodeGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    bool hasDeg90  = node()->disabledContact() != 1;
    bool hasDeg180 = node()->disabledContact() != 2;
    bool hasDeg270 = node()->disabledContact() != 3;

    connectors.emplace_back(location(), rotate(), 0); // Common

    if(hasDeg90)
        connectors.emplace_back(location(), rotate() + TileRotate::Deg90, 1);

    if(hasDeg180)
        connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 2);

    if(hasDeg270)
        connectors.emplace_back(location(), rotate() + TileRotate::Deg270, 3);
}

SimpleCircuitNode *SimpleNodeGraphItem::node() const
{
    return static_cast<SimpleCircuitNode *>(getAbstractNode());
}
