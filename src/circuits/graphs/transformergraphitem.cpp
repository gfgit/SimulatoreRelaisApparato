/**
 * src/circuits/graphs/transformergraphitem.cpp
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

#include "transformergraphitem.h"

#include "../nodes/transformernode.h"
#include "../circuitscene.h"

#include "circuitcolors.h"

#include <QPainter>


TransformerGraphItem::TransformerGraphItem(TransformerNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void TransformerGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    QLineF commonLine;
    QLineF contact1Line;

    QPointF coilCenters[2];

    const auto cableDirection = toConnectorDirection(rotate());
    switch (cableDirection)
    {
    case Connector::Direction::South:
    case Connector::Direction::North:
        // From center to South
        commonLine.setP1(QPointF(TileLocation::HalfSize,
                                 TileLocation::Size - WireLength));
        commonLine.setP2(QPointF(TileLocation::HalfSize,
                                 TileLocation::Size));

        // From center to North
        contact1Line.setP1(QPointF(TileLocation::HalfSize,
                                   0 + WireLength));
        contact1Line.setP2(QPointF(TileLocation::HalfSize, 0));

        coilCenters[0] = commonLine.p1() + QPointF(0, -CoilRadius);
        coilCenters[1] = contact1Line.p1() + QPointF(0, +CoilRadius);

        if(cableDirection == Connector::Direction::North)
        {
            std::swap(commonLine, contact1Line);
            std::swap(coilCenters[0], coilCenters[1]);
        }

        break;

    case Connector::Direction::East:
    case Connector::Direction::West:
        // From center to East
        commonLine.setP1(QPointF(TileLocation::Size - WireLength,
                                 TileLocation::HalfSize));
        commonLine.setP2(QPointF(TileLocation::Size,
                                 TileLocation::HalfSize));

        // From center to West
        contact1Line.setP1(QPointF(0 + WireLength,
                                   TileLocation::HalfSize));
        contact1Line.setP2(QPointF(0, TileLocation::HalfSize));

        coilCenters[0] = commonLine.p1() + QPointF(-CoilRadius, 0);
        coilCenters[1] = contact1Line.p1() + QPointF(+CoilRadius, 0);

        if(cableDirection == Connector::Direction::West)
        {
            std::swap(commonLine, contact1Line);
            std::swap(coilCenters[0], coilCenters[1]);
        }

        break;
    default:
        break;
    }

    // Draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    // Draw common contact (1)
    if(circuitScene()->mode() == FileMode::Editing)
        pen.setColor(Qt::cyan); // Draw contact 1 blue during Editing
    else
        pen.setColor(getContactColor(1));
    painter->setPen(pen);
    painter->drawLine(contact1Line);
    painter->drawEllipse(coilCenters[1], CoilRadius, CoilRadius);

    // Draw common contact (0) on top
    pen.setColor(getContactColor(0));
    painter->setPen(pen);
    painter->drawLine(commonLine);
    painter->drawEllipse(coilCenters[0], CoilRadius, CoilRadius);
}

void TransformerGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 1);
}

TransformerNode *TransformerGraphItem::node() const
{
    return static_cast<TransformerNode *>(getAbstractNode());
}
