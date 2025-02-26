/**
 * src/circuits/graphs/relaiscontactgraphitem.cpp
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

#include "relaiscontactgraphitem.h"

#include "../nodes/relaiscontactnode.h"
#include "../../objects/relais/model/abstractrelais.h"

#include "../../views/modemanager.h"

#include <QPainter>

RelaisContactGraphItem::RelaisContactGraphItem(RelaisContactNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

}

void RelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    bool contactUpOn = node()->isContactOn(AbstractDeviatorNode::UpIdx);
    bool contactDownOn = node()->isContactOn(AbstractDeviatorNode::DownIdx);

    if(node()->relais() &&
            node()->modeMgr()->mode() != FileMode::Simulation)
    {
        // In static or editing mode,
        // draw relay in its normal state
        // unlsess user choses to hide state for this node
        RelaisContactNode::State state = RelaisContactNode::State::Down;

        if(node()->relais() && node()->relais()->normallyUp())
        {
            if(!node()->hideRelayNormalState())
                state = RelaisContactNode::State::Up;
        }

        contactUpOn = state == RelaisContactNode::State::Up;
        contactDownOn = state == RelaisContactNode::State::Down;
        if(node()->swapContactState())
            std::swap(contactUpOn, contactDownOn);
    }

    drawDeviator(painter, contactUpOn, contactDownOn);

    // Draw name
    if(node()->relais())
    {
        QColor color = Qt::black;

        if(node()->modeMgr()->mode() == FileMode::Simulation)
        {
            // Draw name in red for active relais only during simulation mode
            // (Stabilized relay stay active also in editing mode)
            switch (node()->relais()->state())
            {
            case AbstractRelais::State::Up:
                color = Qt::red;
                break;
            case AbstractRelais::State::GoingUp:
            case AbstractRelais::State::GoingDown:
                color.setRgb(120, 210, 255); // Light blue
                break;
            case AbstractRelais::State::Down:
            default:
                break;
            }
        }

        painter->setPen(color);

        TileRotate nameRotate = rotate();
        if(node()->flipContact())
            nameRotate += TileRotate::Deg180;

        QRectF textBr;
        drawName(painter,
                 node()->relais() ? node()->relais()->name() : tr("REL!"),
                 nameRotate, &textBr);

        if(!node()->hideRelayNormalState())
            drawRelayArrow(painter, nameRotate, textBr);
    }
}

QString RelaisContactGraphItem::displayString() const
{
    if(node()->relais())
        return node()->relais()->name();
    return QLatin1String("REL!");
}

QString RelaisContactGraphItem::tooltipString() const
{
    if(!node()->relais())
        return tr("No Relay set!");

    return tr("Contact of relay <b>%1</b><br>"
              "State: <b>%2</b><br>"
              "%3")
            .arg(node()->relais()->name(),
                 node()->relais()->getStateName(),
                 getContactTooltip());
}

void RelaisContactGraphItem::drawRelayArrow(QPainter *painter,
                                            TileRotate r,
                                            const QRectF& textBr)
{
    if(!node()->relais())
        return;

    if(node()->relais()->relaisType() == AbstractRelais::RelaisType::Combinator)
        return; // Combinators do not need arrow

    if(node()->relais()->state() == AbstractRelais::State::GoingUp
            || node()->relais()->state() == AbstractRelais::State::GoingDown)
        return; // Do not draw arrow for transitory states

    // Draw arrow up/down for normally up/down relays
    QRectF arrowRect;

    // Follow drawName() layout to place arrown next to name
    switch (toConnectorDirection(r - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        arrowRect.setLeft(textBr.right() + 5.0);
        arrowRect.setWidth(10.0);
        arrowRect.setTop(10.0);
        arrowRect.setBottom(TileLocation::HalfSize - 12.0);
        break;

    case Connector::Direction::South:
        arrowRect.setLeft(textBr.right() + 5.0);
        arrowRect.setWidth(10.0);
        arrowRect.setTop(TileLocation::HalfSize + 12.0);
        arrowRect.setBottom(TileLocation::Size - 10.0);
        break;

    case Connector::Direction::East:
        arrowRect.setLeft(TileLocation::HalfSize + 3.0);
        arrowRect.setRight(TileLocation::Size - 10.0);
        arrowRect.setTop(TileLocation::HalfSize + 2.0);
        arrowRect.setBottom(TileLocation::Size - 20.0);
        break;

    case Connector::Direction::West:
        arrowRect.setLeft(10.0);
        arrowRect.setRight(TileLocation::HalfSize - 3.0);
        arrowRect.setTop(TileLocation::HalfSize + 2.0);
        arrowRect.setBottom(TileLocation::Size - 20.0);
        break;

    default:
        break;
    }

    // Don't draw out of item's tile rect
    if(arrowRect.right() > TileLocation::Size)
        arrowRect.setRight(TileLocation::Size);

    QLineF line;
    QPointF triangle[3];

    const double centerX = arrowRect.center().x();
    const double lineHeight = arrowRect.height() * 0.55;

    const double triangleSemiWidth = 0.5 * qMin(arrowRect.width(),
                                                arrowRect.height() - lineHeight);

    bool isRelayUp = node()->relais()->state() == AbstractRelais::State::Up;
    if(node()->modeMgr()->mode() != FileMode::Simulation)
    {
        // In static or editing mode,
        // draw relay in its normal state
        isRelayUp = node()->relais()->normallyUp();
    }

    if(isRelayUp)
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

    /* Colors:
     * Black: relay is normally down and currently down
     * Red: relay is normally up and currently up
     *
     * Blue: relay is normally up BUT currently down
     * Purple: relay is normally down BUT currently up
     */

    QColor color = Qt::black;
    if(isRelayUp)
    {
        if(node()->relais()->normallyUp())
            color = Qt::red; // Relay in normal state
        else
            color = Qt::darkMagenta;
    }
    else
    {
        if(!node()->relais()->normallyUp())
            color = Qt::black; // Relay in normal state
        else
            color = Qt::blue;
    }

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(3.5);
    pen.setColor(color);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(line);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(triangle, 3);
}

RelaisContactNode *RelaisContactGraphItem::node() const
{
    return static_cast<RelaisContactNode *>(getAbstractNode());
}
