/**
 * src/circuits/graphs/onoffgraphitem.cpp
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

#include "onoffgraphitem.h"

#include "../nodes/onoffswitchnode.h"
#include "../circuitscene.h"

#include "circuitcolors.h"

#include <QPainter>

OnOffGraphItem::OnOffGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &OnOffSwitchNode::isOnChanged,
            this, &OnOffGraphItem::triggerUpdate);
}

void OnOffGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    QLineF commonLine;
    QLineF contact1Line;
    QLineF switchLine;

    double endOffset = node()->isOn() ? 0.0 : 15.0;

    switch (toConnectorDirection(rotate() - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        switchLine.setP1(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize - 15));
        switchLine.setP2(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize + endOffset));
        break;

    case Connector::Direction::South:
        switchLine.setP1(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize - endOffset));
        switchLine.setP2(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize + 15));
        break;

    case Connector::Direction::East:
        switchLine.setP1(QPointF(TileLocation::HalfSize - endOffset,
                                 TileLocation::HalfSize + 15));
        switchLine.setP2(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize + 15));
        break;

    case Connector::Direction::West:
        switchLine.setP1(QPointF(TileLocation::HalfSize - 15,
                                 TileLocation::HalfSize + 15));
        switchLine.setP2(QPointF(TileLocation::HalfSize + endOffset,
                                 TileLocation::HalfSize + 15));
        break;
    default:
        break;
    }

    const auto cableDirection = toConnectorDirection(rotate());
    switch (cableDirection)
    {
    case Connector::Direction::South:
    case Connector::Direction::North:
        // From switch to South
        commonLine.setP1(QPointF(TileLocation::HalfSize,
                                 TileLocation::HalfSize + 15));
        commonLine.setP2(QPointF(TileLocation::HalfSize,
                                 TileLocation::Size - 22.0));

        // From switch to North
        contact1Line.setP1(commonLine.p1());
        contact1Line.setP2(QPointF(TileLocation::HalfSize, 22.0));

        if(cableDirection == Connector::Direction::North)
            std::swap(commonLine, contact1Line);

        break;

    case Connector::Direction::East:
    case Connector::Direction::West:
        // From switch to East
        commonLine.setP1(QPointF(TileLocation::HalfSize + 15,
                                 TileLocation::HalfSize));
        commonLine.setP2(QPointF(TileLocation::Size - 22.0,
                                 TileLocation::HalfSize));

        // From switch to West
        contact1Line.setP1(commonLine.p1());
        contact1Line.setP2(QPointF(22.0, TileLocation::HalfSize));

        if(cableDirection == Connector::Direction::West)
            std::swap(commonLine, contact1Line);
        break;
    default:
        break;
    }

    // Draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);

    // Draw common contact (0)
    pen.setColor(getContactColor(0));
    painter->setPen(pen);
    painter->drawLine(commonLine);

    // Draw first contact (1)
    pen.setColor(getContactColor(1));
    painter->setPen(pen);
    painter->drawLine(contact1Line);

    // Draw switch line
    if(!node()->isOn())
    {
        // Switch is off or no circuit passes through
        // Otherwise turned on if both sides are on, and switch is on
        pen.setColor(CircuitColors::None);
    }

    // Use square cap for switch line to cover
    // contact line fully
    pen.setCapStyle(Qt::SquareCap);
    painter->setPen(pen);
    painter->drawLine(switchLine);

    drawMorsetti(painter, 0, rotate());
    drawMorsetti(painter, 1, rotate() + TileRotate::Deg180);

    painter->setPen(node()->isOn() ? Qt::red : Qt::black);
    drawName(painter, node()->objectName(), rotate());
}

void OnOffGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev)
{
    AbstractNodeGraphItem::mouseDoubleClickEvent(ev);

    CircuitScene *s = circuitScene();
    if(s && s->mode() == FileMode::Simulation)
    {
        // Toggle on double click
        bool val = node()->isOn();
        node()->setOn(!val);
    }
}

void OnOffGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 1);
}

OnOffSwitchNode *OnOffGraphItem::node() const
{
    return static_cast<OnOffSwitchNode *>(getAbstractNode());
}
