/**
 * src/circuits/graphs/remotecablecircuitgraphitem.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#include "remotecablecircuitgraphitem.h"
#include "../nodes/remotecablecircuitnode.h"

#include <QPainter>

RemoteCableCircuitGraphItem::RemoteCableCircuitGraphItem(RemoteCableCircuitNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RemoteCableCircuitNode::modeChanged,
            this, &RemoteCableCircuitGraphItem::triggerUpdate);
}

void RemoteCableCircuitGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    QLineF commonLine;

    const auto cableDirection = toConnectorDirection(rotate());
    switch (cableDirection)
    {
    case Connector::Direction::South:
    case Connector::Direction::North:
        // From center to South
        commonLine.setP1(QPointF(TileLocation::HalfSize,
                                 TileLocation::HalfSize));
        commonLine.setP2(QPointF(TileLocation::HalfSize,
                                 TileLocation::Size));

        // From center to North
        if(cableDirection == Connector::Direction::North)
            commonLine.setP2(QPointF(TileLocation::HalfSize,
                                     0));

        break;

    case Connector::Direction::East:
    case Connector::Direction::West:
        // From center to East
        commonLine.setP1(QPointF(TileLocation::HalfSize,
                                 TileLocation::HalfSize));
        commonLine.setP2(QPointF(TileLocation::Size,
                                 TileLocation::HalfSize));

        // From center to West
        if(cableDirection == Connector::Direction::West)
            commonLine.setP2(QPointF(0, TileLocation::HalfSize));
        break;
    default:
        break;
    }

    const QColor colors[3] =
    {
        QColor(120, 210, 255), // Light blue, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    // Draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(5.0);
    pen.setCapStyle(Qt::FlatCap);
    pen.setStyle(Qt::DashLine);

    // Draw common line dashed (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    painter->drawLine(commonLine);

    // Draw description below cable
    // unless cable is in Center-South direction
    TileRotate textRotate = TileRotate::Deg90;
    if(rotate() == TileRotate::Deg0)
        textRotate = TileRotate::Deg270; // Description above

    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    drawName(painter,
             node()->getDescription(),
             textRotate);
}

void RemoteCableCircuitGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

RemoteCableCircuitNode *RemoteCableCircuitGraphItem::node() const
{
    return static_cast<RemoteCableCircuitNode *>(getAbstractNode());
}
