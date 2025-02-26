/**
 * src/circuits/graphs/electromagnetgraphitem.cpp
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

#include "electromagnetgraphitem.h"

#include "../nodes/electromagnetnode.h"

#include "../../objects/simple_activable/abstractsimpleactivableobject.h"

#include <QPainter>

ElectroMagnetGraphItem::ElectroMagnetGraphItem(ElectroMagnetNode *node_)
    : SimpleActivationGraphItem(node_)
{

}

void ElectroMagnetGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    SimpleActivationGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double bulbSize = 32.0;
    constexpr double centerOffset = bulbSize / 2.0;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;
    QRectF bulbRect;
    bulbRect.setSize(QSizeF(circleRadius * 2.0, circleRadius * 2.0));
    bulbRect.moveCenter(center);

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        break;
    default:
        break;
    }

    //drawMorsetti(painter, 0, rotate() + TileRotate::Deg0);

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    const QColor colors[3] =
    {
        QColor(120, 210, 255), // Light blue, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    // Draw common contact (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    //painter->drawLine(commonLine);

    // Draw bulb circle

    pen.setWidthF(10.0);
    painter->setPen(pen);

    if(node()->object() &&
            node()->object()->state() == AbstractSimpleActivableObject::State::On)
        pen.setColor(colors[int(AnyCircuitType::Closed)]);
    else
        pen.setColor(colors[int(AnyCircuitType::None)]);

    painter->drawEllipse(bulbRect);

    // TileRotate textRotate = TileRotate::Deg90;
    // if(rotate() == TileRotate::Deg0)
    //     textRotate = TileRotate::Deg270;

    // drawName(painter,
    //          node()->object() ? node()->object()->name() : tr("OBJ?"),
    //          textRotate);

    drawName(painter);
}

ElectroMagnetNode *ElectroMagnetGraphItem::node() const
{
    return static_cast<ElectroMagnetNode *>(getAbstractNode());
}
