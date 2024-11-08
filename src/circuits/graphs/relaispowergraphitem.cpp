/**
 * src/circuits/graphs/relaispowergraphitem.cpp
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
#include "../../objects/relais/model/abstractrelais.h"

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
    constexpr double centerOffset = relayRadius;

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
    relayRect.setSize(QSizeF(relayRadius * 2.0, relayRadius * 2.0));
    relayRect.moveCenter(center);

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

    drawMorsetti(painter, 0, rotate() + TileRotate::Deg0);

    // Now draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(5.0);
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
    painter->drawLine(commonLine);

    QColor color = colors[int(AnyCircuitType::None)]; // Black
    if(node()->relais())
    {
        switch (node()->relais()->state())
        {
        case AbstractRelais::State::Up:
            color = colors[int(AnyCircuitType::Closed)]; // Red
            break;
        case AbstractRelais::State::GoingUp:
        case AbstractRelais::State::GoingDown:
            color = colors[int(AnyCircuitType::Open)]; // Light blue
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

    // Draw delayed up/down
    if(node()->delayUpSeconds() > 0)
    {
        painter->drawLine(relayRect.topLeft(), relayRect.topRight());
    }
    if(node()->delayDownSeconds() > 0)
    {
        painter->drawLine(relayRect.bottomLeft(), relayRect.bottomRight());
    }

    // Draw state arrow
    TileRotate arrowRotate = TileRotate::Deg90;
    if(rotate() == TileRotate::Deg90)
        arrowRotate = TileRotate::Deg270;
    drawRelayArrow(painter, arrowRotate, color);

    // Draw name
    TileRotate textRotate = TileRotate::Deg90;
    if(rotate() == TileRotate::Deg0)
        textRotate = TileRotate::Deg270;

    drawName(painter,
             node()->relais() ? node()->objectName() : tr("REL!"),
             textRotate);
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
    update();
}

void RelaisPowerGraphItem::drawRelayArrow(QPainter *painter,
                                          TileRotate r,
                                          const QColor& color)
{
    if(!node()->relais())
        return;

    // Draw arrow up/down for normally up/down relays
    QRectF arrowRect;

    switch (toConnectorDirection(r))
    {
    case Connector::Direction::East:
        arrowRect.setLeft(TileLocation::HalfSize + relayRadius + 5.0);
        arrowRect.setRight(TileLocation::Size - 5.0);
        arrowRect.setTop(TileLocation::HalfSize - relayRadius);
        arrowRect.setBottom(TileLocation::HalfSize + relayRadius);
        break;

    case Connector::Direction::West:
        arrowRect.setLeft(5.0);
        arrowRect.setRight(TileLocation::HalfSize - relayRadius - 5.0);
        arrowRect.setTop(TileLocation::HalfSize - relayRadius);
        arrowRect.setBottom(TileLocation::HalfSize + relayRadius);
        break;
    default:
        break;
    }

    QLineF line;
    QPointF triangle[3];

    const double centerX = arrowRect.center().x();
    const double lineHeight = arrowRect.height() * 0.6;

    const double triangleSemiWidth = 0.5 * qMin(arrowRect.width(),
                                                arrowRect.height() - lineHeight);

    if(node()->relais()->normallyUp())
    {
        // Arrow up
        line.setP1(QPointF(centerX, arrowRect.bottom() - lineHeight));
        line.setP2(QPointF(centerX, arrowRect.bottom()));

        triangle[0] = QPointF(centerX, arrowRect.top());
        triangle[1] = QPointF(centerX + triangleSemiWidth, line.y1());
        triangle[2] = QPointF(centerX - triangleSemiWidth, line.y1());
    }
    else
    {
        // Arrow down
        line.setP1(QPointF(centerX, arrowRect.top() + lineHeight));
        line.setP2(QPointF(centerX, arrowRect.top()));

        triangle[0] = QPointF(centerX, arrowRect.bottom());
        triangle[1] = QPointF(centerX + triangleSemiWidth, line.y1());
        triangle[2] = QPointF(centerX - triangleSemiWidth, line.y1());
    }

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(3.0);
    pen.setColor(color);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(line);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(triangle, 3);
}

RelaisPowerNode *RelaisPowerGraphItem::node() const
{
    return static_cast<RelaisPowerNode *>(getAbstractNode());
}
