/**
 * src/circuits/graphs/resistorgraphitem.cpp
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

#include "resistorgraphitem.h"

#include "../nodes/resistornode.h"
#include "../circuitscene.h"

#include "circuitcolors.h"

#include <QPainter>


ResistorGraphItem::ResistorGraphItem(ResistorNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void ResistorGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double centerOffset = 5;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), 0);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  0, center.y());

    QLineF commonLine;
    QLineF contact1Line;

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        contact1Line = centerToSouth;
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        contact1Line = centerToNorth;
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        contact1Line = centerToWest;
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        contact1Line = centerToEast;
        break;
    default:
        break;
    }

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    // Draw common contact (0)
    pen.setColor(getContactColor(0));
    painter->setPen(pen);
    painter->drawLine(commonLine);

    // Draw fisrt contact (1)
    painter->drawLine(contact1Line);
}

void ResistorGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 1);
}

ResistorNode *ResistorGraphItem::node() const
{
    return static_cast<ResistorNode *>(getAbstractNode());
}
