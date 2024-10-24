/**
 * src/graph/relaispowergraphitem.cpp
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

#include "relaispowergraphitem.h"

#include "../nodes/relaispowernode.h"
#include "../objects/abstractrelais.h"

#include <QPainter>

RelaisPowerGraphItem::RelaisPowerGraphItem(RelaisPowerNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RelaisPowerNode::relayChanged,
            this, &RelaisPowerGraphItem::updateRelay);
    updateRelay();
}

void RelaisPowerGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);
    constexpr double morsettiOffset = 22.0;
    constexpr double centerOffset = 22.0;
    constexpr double relaySize = 38.0;

    constexpr QLineF centerToNorth(center.x(), center.y() - centerOffset,
                                   center.x(), morsettiOffset);

    constexpr QLineF centerToSouth(center.x(), center.y() + centerOffset,
                                   center.x(), TileLocation::Size - morsettiOffset);

    constexpr QLineF centerToEast(center.x() + centerOffset, center.y(),
                                  TileLocation::Size - morsettiOffset, center.y());

    constexpr QLineF centerToWest(center.x() - centerOffset, center.y(),
                                  morsettiOffset, center.y());

    QLineF commonLine;
    QRectF relayRect;
    relayRect.setSize(QSizeF(relaySize, relaySize));
    relayRect.moveCenter(center);

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        commonLine = centerToNorth;
        relayRect.moveTop(commonLine.y1());
        break;

    case Connector::Direction::South:
        commonLine = centerToSouth;
        relayRect.moveBottom(commonLine.y1());
        break;

    case Connector::Direction::East:
        commonLine = centerToEast;
        relayRect.moveRight(commonLine.x1());
        break;

    case Connector::Direction::West:
        commonLine = centerToWest;
        relayRect.moveLeft(commonLine.x1());
        break;
    default:
        break;
    }

    drawMorsetti(painter, 0, rotate() + TileRotate::Deg0);

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(5.0);
    pen.setCapStyle(Qt::FlatCap);

    const QColor colors[3] =
    {
        QColor(255, 140, 140), // Light red, Open Circuit
        Qt::red, // Closed circuit
        Qt::black // No circuit
    };

    // Draw common contact (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    painter->drawLine(commonLine);

    QColor color = colors[2];
    if(node()->relais())
    {
        switch (node()->relais()->state())
        {
        case AbstractRelais::State::Up:
            color = colors[0];
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color = colors[1]; // Light red
            break;
        case AbstractRelais::State::Down:
        default:
            break;
        }
    }

    pen.setWidthF(3.0);
    pen.setColor(color);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Draw circle
    painter->drawEllipse(relayRect);

    TileRotate textRotate = TileRotate::Deg90;
    if(rotate() == TileRotate::Deg0)
        textRotate = TileRotate::Deg270;

    if(node()->relais())
        drawName(painter, node()->objectName(), textRotate);
}

void RelaisPowerGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

void RelaisPowerGraphItem::updateRelay()
{
    if(mRelay == node()->relais())
        return;

    if(mRelay)
    {
        disconnect(mRelay, &AbstractRelais::stateChanged,
                   this, &RelaisPowerGraphItem::triggerUpdate);
    }

    mRelay = node()->relais();

    if(mRelay)
    {
        connect(mRelay, &AbstractRelais::stateChanged,
                this, &RelaisPowerGraphItem::triggerUpdate);
    }

    update();
}

void RelaisPowerGraphItem::updateName()
{
    setToolTip(mRelay ?
                   mRelay->objectName() :
                   QLatin1String("NO RELAY SET"));
}

RelaisPowerNode *RelaisPowerGraphItem::node() const
{
    return static_cast<RelaisPowerNode *>(getAbstractNode());
}
