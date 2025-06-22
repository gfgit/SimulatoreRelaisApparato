/**
 * src/circuits/graphs/lightbulbgraphitem.cpp
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

#include "lightbulbgraphitem.h"

#include "../nodes/lightbulbnode.h"

#include "../../objects/simple_activable/abstractsimpleactivableobject.h"

#include "circuitcolors.h"

#include <QPainter>

LightBulbGraphItem::LightBulbGraphItem(LightBulbNode *node_)
    : SimpleActivationGraphItem(node_)
{

}

void LightBulbGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    SimpleActivationGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double centerOffset = circleRadius;

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
        CircuitColors::Open,
        CircuitColors::Closed,
        CircuitColors::None
    };

    // Draw common contact (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    // painter->drawLine(commonLine);

    // Draw bulb circle

    // If we have only open circuit (light bulb is still off)
    // Draw in black instead of Light blue.
    if(!node()->hasCircuits())
        pen.setColor(colors[int(AnyCircuitType::None)]);
    pen.setWidthF(10.0);
    painter->setPen(pen);

    if(node()->object() &&
            node()->object()->state() == AbstractSimpleActivableObject::State::On)
        painter->setBrush(Qt::yellow);
    else
        painter->setBrush(Qt::NoBrush);

    painter->drawEllipse(bulbRect);

    // Draw a cross onto the circle
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(bulbRect.topLeft(), bulbRect.bottomRight());
    painter->drawLine(bulbRect.topRight(), bulbRect.bottomLeft());

    // TileRotate textRotate = TileRotate::Deg90;
    // if(rotate() == TileRotate::Deg0)
    //     textRotate = TileRotate::Deg270;

    // drawName(painter,
    //          node()->object() ? node()->object()->name() : tr("OBJ?"),
    //          textRotate);

    drawName(painter);
}

LightBulbNode *LightBulbGraphItem::node() const
{
    return static_cast<LightBulbNode *>(getAbstractNode());
}
