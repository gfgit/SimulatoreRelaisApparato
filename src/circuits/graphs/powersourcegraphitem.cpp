/**
 * src/circuits/graphs/powersourcegraphitem.cpp
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

#include "powersourcegraphitem.h"

#include "../nodes/powersourcenode.h"
#include "../circuitscene.h"

#include <QPainterPath>
#include <QPainter>

#include <QGraphicsSceneMouseEvent>

PowerSourceGraphItem::PowerSourceGraphItem(PowerSourceNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &PowerSourceNode::enabledChanged,
            this, &PowerSourceGraphItem::triggerUpdate);
}

void PowerSourceGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(node()->isSourceEnabled() ? Qt::red : Qt::darkGreen);

    // Draw a triangle
    const QPointF *triangle = nullptr;
    constexpr QPointF north[3] = {{20, 82}, {80, 82}, {50, 22}};
    constexpr QPointF south[3] = {{80, 18}, {20, 18}, {50, 78}};
    constexpr QPointF east[3] = {{18, 80}, {18, 20}, {78, 50}};
    constexpr QPointF west[3] = {{82, 20}, {82, 80}, {22, 50}};

    switch (toConnectorDirection(rotate()))
    {
    case Connector::Direction::North:
        triangle = north;
        break;

    case Connector::Direction::South:
        triangle = south;
        break;

    case Connector::Direction::East:
        triangle = east;
        break;

    case Connector::Direction::West:
        triangle = west;
        break;
    default:
        break;
    }

    if(triangle)
        painter->drawConvexPolygon(triangle, 3);

    drawMorsetti(painter, 0, rotate());
}

void PowerSourceGraphItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev)
{
    AbstractNodeGraphItem::mouseDoubleClickEvent(ev);

    CircuitScene *s = circuitScene();
    if(s && s->mode() == FileMode::Simulation &&
            baseTileRect().contains(ev->pos()) &&
            ev->button() == Qt::LeftButton)
    {
        // Toggle on double click
        bool val = node()->isSourceEnabled();
        node()->setSourceEnabled(!val);
    }
}

void PowerSourceGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

PowerSourceNode *PowerSourceGraphItem::node() const
{
    return static_cast<PowerSourceNode *>(getAbstractNode());
}
