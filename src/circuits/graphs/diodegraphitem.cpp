/**
 * src/circuits/graphs/diodegraphitem.cpp
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

#include "diodegraphitem.h"

#include "../nodes/diodecircuitnode.h"

#include "circuitcolors.h"

#include <QPainter>

DiodeGraphItem::DiodeGraphItem(DiodeCircuitNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void DiodeGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double centerOffset = 20;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF anodeLine;
    QLineF cathodeLine;

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        anodeLine = centerToNorth;
        cathodeLine = centerToSouth;
        break;

    case Connector::Direction::South:
        anodeLine = centerToSouth;
        cathodeLine = centerToNorth;
        break;

    case Connector::Direction::East:
        anodeLine = centerToEast;
        cathodeLine = centerToWest;
        break;

    case Connector::Direction::West:
        anodeLine = centerToWest;
        cathodeLine = centerToEast;
        break;
    default:
        break;
    }

    drawMorsetti(painter, 0, rotate());
    drawMorsetti(painter, 1, rotate() + TileRotate::Deg180);

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    const QColor colors[3] =
    {
        CircuitColors::Open,
        CircuitColors::Closed,
        CircuitColors::None
    };

    // Draw common contact (Anode, 0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    painter->drawLine(anodeLine);

    // Draw second contact (Cathode, 1)
    pen.setColor(colors[int(node()->hasAnyCircuit(1))]);
    painter->setPen(pen);
    painter->drawLine(cathodeLine);

    // Draw diode Triangle
    QPointF triangle[3];
    QLineF diodeLine;
    const double diodeHeight = centerOffset / 0.86;

    if(qFuzzyCompare(cathodeLine.y1(), anodeLine.y1()))
    {
        // Horizontal
        triangle[0] = cathodeLine.p1();
        triangle[1] = {anodeLine.x1(), anodeLine.y1() + diodeHeight};
        triangle[2] = {anodeLine.x1(), anodeLine.y1() - diodeHeight};

        diodeLine = QLineF(cathodeLine.x1(), center.y() + diodeHeight,
                           cathodeLine.x1(), center.y() - diodeHeight);
    }
    else
    {
        // Vertical
        triangle[0] = cathodeLine.p1();
        triangle[1] = {anodeLine.x1() + diodeHeight, anodeLine.y1()};
        triangle[2] = {anodeLine.x1() - diodeHeight, anodeLine.y1()};

        diodeLine = QLineF(center.x() + diodeHeight, cathodeLine.y1(),
                           center.x() - diodeHeight, cathodeLine.y1());
    }

    // Color triangle only if circuits are passing through
    QColor passingColor = colors[int(node()->hasAnyPassingCircuits())];

    // Triangle
    painter->setBrush(passingColor);
    painter->setPen(Qt::NoPen);

    painter->drawPolygon(triangle, 3);

    // Diode Line
    painter->setBrush(Qt::NoBrush);
    pen.setColor(passingColor);
    painter->setPen(pen);

    painter->drawLine(diodeLine);
}

void DiodeGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 1);
}

DiodeCircuitNode *DiodeGraphItem::node() const
{
    return static_cast<DiodeCircuitNode *>(getAbstractNode());
}
