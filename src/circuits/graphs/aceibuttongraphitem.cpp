/**
 * src/circuits/graphs/aceibuttongraphitem.cpp
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

#include "aceibuttongraphitem.h"

#include "../nodes/aceibuttonnode.h"

#include "../circuitscene.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

ACEIButtonGraphItem::ACEIButtonGraphItem(ACEIButtonNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

}

void ACEIButtonGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    drawDeviator(painter,
                 node()->isContactOn(AbstractDeviatorNode::UpIdx),
                 node()->isContactOn(AbstractDeviatorNode::DownIdx));

    // Draw name
    QColor color = Qt::black;

    switch (node()->state())
    {
    case ACEIButtonNode::State::Pressed:
        color = Qt::darkGreen;
        break;
    case ACEIButtonNode::State::Extracted:
        color = Qt::red;
        break;
    case ACEIButtonNode::State::Normal:
    default:
        break;
    }

    painter->setPen(color);

    TileRotate nameRotate = rotate();
    if(node()->flipContact())
        nameRotate += TileRotate::Deg180;
    drawName(painter, node()->objectName(), nameRotate);
}

ACEIButtonNode *ACEIButtonGraphItem::node() const
{
    return static_cast<ACEIButtonNode *>(getAbstractNode());
}

void ACEIButtonGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->mode() == FileMode::Simulation)
    {
        if(ev->button() == Qt::LeftButton)
            node()->setState(ACEIButtonNode::State::Pressed);
        else if(ev->button() == Qt::RightButton)
            node()->setState(ACEIButtonNode::State::Extracted);
        return;
    }

    AbstractNodeGraphItem::mousePressEvent(ev);
}

void ACEIButtonGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    CircuitScene *s = circuitScene();
    if(s && s->mode() == FileMode::Simulation)
    {
        if(ev->button() == Qt::LeftButton || ev->button() == Qt::RightButton)
            node()->setState(ACEIButtonNode::State::Normal);
        return;
    }

    AbstractNodeGraphItem::mouseReleaseEvent(ev);
}

void ACEIButtonGraphItem::drawCustomArc(QPainter *painter,
                                              const QLineF &contact1Line,
                                              const QLineF &contact2Line,
                                              const QPointF &center)
{
    // Custom arc diagonal line for ACEI Buttons

    // Draw black middle diagonal line below everything
    QPointF corner;
    corner.setX(contact1Line.x2());
    corner.setY(contact1Line.y2());
    if(qFuzzyCompare(contact1Line.x2(), center.x()))
        corner.setX(contact2Line.x2());
    if(qFuzzyCompare(contact1Line.y2(), center.y()))
        corner.setY(contact2Line.y2());
    painter->drawLine(center, corner);
}
