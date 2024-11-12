/**
 * src/circuits/graphs/polarityinversiongraphitem.cpp
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

#include "polarityinversiongraphitem.h"

#include "../nodes/polarityinversionnode.h"

#include <QPainter>

PolarityInversionGraphItem::PolarityInversionGraphItem(PolarityInversionNode *node_)
    : AbstractNodeGraphItem{node_}
{

}

void PolarityInversionGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    // We do not draw morsetti on this node

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    constexpr double crossSize = 30.0;
    constexpr double centerOffset = crossSize / 2.0;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), 0);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  0, center.y());

    QLineF connectorLine1;
    QLineF connectorLine2;
    bool horizontal = false;

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        connectorLine1 = centerToNorth;
        connectorLine2 = centerToSouth;
        horizontal = false;
        break;

    case Connector::Direction::South:
        connectorLine1 = centerToSouth;
        connectorLine2 = centerToNorth;
        horizontal = false;
        break;

    case Connector::Direction::East:
        connectorLine1 = centerToEast;
        connectorLine2 = centerToWest;
        horizontal = true;
        break;

    case Connector::Direction::West:
        connectorLine1 = centerToWest;
        connectorLine2 = centerToEast;
        horizontal = true;
        break;

    default:
        break;
    }

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(5.0);
    pen.setCapStyle(Qt::FlatCap);

    // Fill edges with miter join
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    const QColor colors[3] =
    {
        QColor(120, 210, 255), // Light blue, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);

    // TODO: polyline for join
    painter->drawLine(connectorLine1);


    QPointF arr[3];
    arr[0] = connectorLine1.p2();
    arr[1] = connectorLine1.p1();
    if(horizontal)
    {
        arr[2].rx() = connectorLine2.x1();
        arr[2].ry() = center.y() + crossSize * (connectorLine1.x1() > connectorLine2.x1() ? 1 : -1);
    }
    else
    {
        arr[2].ry() = connectorLine2.y1();
        arr[2].rx() = center.x() + crossSize * (connectorLine1.y1() < connectorLine2.y1() ? 1 : -1);
    }

    painter->drawPolyline(arr, 3);

    arr[0] = connectorLine2.p2();
    arr[1] = connectorLine2.p1();
    if(horizontal)
    {
        arr[2].rx() = connectorLine1.x1();
        arr[2].ry() = center.y() + crossSize * (connectorLine1.x1() > connectorLine2.x1() ? 1 : -1);
    }
    else
    {
        arr[2].ry() = connectorLine1.y1();
        arr[2].rx() = center.x() + crossSize * (connectorLine1.y1() < connectorLine2.y1() ? 1 : -1);
    }

    painter->drawPolyline(arr, 3);
}

void PolarityInversionGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 1);
}

PolarityInversionNode *PolarityInversionGraphItem::node() const
{
    return static_cast<PolarityInversionNode *>(getAbstractNode());
}
