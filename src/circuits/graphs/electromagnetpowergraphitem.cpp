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

#include "electromagnetpowergraphitem.h"

#include "../nodes/electromagnetpowernode.h"

#include "../../objects/simple_activable/electromagnet.h"

#include "circuitcolors.h"

#include <QPainter>

ElectroMagnetPowerGraphItem::ElectroMagnetPowerGraphItem(ElectroMagnetPowerNode *node_)
    : SimpleActivationGraphItem(node_)
{

}

void ElectroMagnetPowerGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
        CircuitColors::Open,
        CircuitColors::Closed,
        CircuitColors::None
    };

    // Draw common contact (0)
    pen.setColor(getContactColor(0));
    painter->setPen(pen);
    //painter->drawLine(commonLine);

    // Draw bulb circle

    pen.setWidthF(10.0);
    pen.setColor(CircuitColors::None);

    ElectroMagnetObject *magnet = static_cast<ElectroMagnetObject *>(node()->object());
    if(magnet)
    {
        const auto curState = magnet->state();
        const auto electricalState = magnet->electricalState();

        if(curState == electricalState)
        {
            if(curState == AbstractSimpleActivableObject::State::On)
                pen.setColor(colors[int(AnyCircuitType::Closed)]);
        }
        else if(curState == AbstractSimpleActivableObject::State::On)
            pen.setColor(qRgb(242, 157, 0)); // Forced up (Orange)
        else if(electricalState == AbstractSimpleActivableObject::State::On)
            pen.setColor(Qt::blue); // Forced down
    }

    painter->setPen(pen);
    painter->drawEllipse(bulbRect);

    // TileRotate textRotate = TileRotate::Deg90;
    // if(rotate() == TileRotate::Deg0)
    //     textRotate = TileRotate::Deg270;

    // drawName(painter,
    //          node()->object() ? node()->object()->name() : tr("OBJ?"),
    //          textRotate);

    drawName(painter);
}

QString ElectroMagnetPowerGraphItem::tooltipString() const
{
    ElectroMagnetObject *magnet = static_cast<ElectroMagnetObject *>(node()->object());
    if(!magnet)
        return SimpleActivationGraphItem::tooltipString();

    const auto curState = magnet->state();
    const auto electricalState = magnet->electricalState();

    QString stateStr;
    if(curState == electricalState)
    {
        if(electricalState == ElectroMagnetObject::State::On)
            stateStr = tr("Up");
        else
            stateStr = tr("Down");
    }
    else
    {
        if(electricalState == ElectroMagnetObject::State::On)
            stateStr = tr("Forced Down (On)");
        else
            stateStr = tr("Forced Up (Off)");
    }

    return tr("Electromagnet <b>%1</b><br>"
              "State: <b>%2</b>")
            .arg(magnet->name(), stateStr);
}

ElectroMagnetPowerNode *ElectroMagnetPowerGraphItem::node() const
{
    return static_cast<ElectroMagnetPowerNode *>(getAbstractNode());
}
