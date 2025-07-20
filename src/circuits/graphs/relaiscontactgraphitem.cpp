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

#include "circuitcolors.h"

#include <QPainter>

RelaisContactGraphItem::RelaisContactGraphItem(RelaisContactNode *node_)
    : AbstractDeviatorGraphItem(node_)
{
    connect(node(), &RelaisContactNode::relayCodeChanged,
            this, &RelaisContactGraphItem::onRelayCodeChanged);
}

void RelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    bool contactUpOn = node()->isContactOn(AbstractDeviatorNode::UpIdx);
    bool contactDownOn = node()->isContactOn(AbstractDeviatorNode::DownIdx);

    AbstractRelais::State relayState = AbstractRelais::State::Down;

    if(node()->relais())
    {
        if(node()->modeMgr()->mode() != FileMode::Simulation)
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
        else
        {
            relayState = node()->relais()->state();

            if(relayState == AbstractRelais::State::Up)
            {
                SignalAspectCode code = SignalAspectCode::CodeAbsent;

                bool forceUp = false;
                bool skip = false;
                switch (node()->relais()->relaisType())
                {
                case AbstractRelais::RelaisType::Encoder:
                {
                    code = node()->relais()->getExpectedCode();
                    break;
                }
                case AbstractRelais::RelaisType::CodeRepeater:
                {
                    code = node()->relais()->getDetectedCode();
                    if(code == SignalAspectCode::CodeAbsent)
                        forceUp = true;
                    break;
                }
                default:
                    skip = true;
                    break;
                }

                if(!skip)
                {
                    // Fake relay pulsing at code frequency
                    if(node()->modeMgr()->getCodePhase(code) || forceUp)
                        relayState = AbstractRelais::State::Up;
                    else
                        relayState = AbstractRelais::State::Down;

                    contactUpOn = relayState == AbstractRelais::State::Up;
                    contactDownOn = relayState == AbstractRelais::State::Down;
                    if(node()->swapContactState())
                        std::swap(contactUpOn, contactDownOn);
                }
            }
        }
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
                color = CircuitColors::Closed; // Red
                break;
            case AbstractRelais::State::GoingUp:
            case AbstractRelais::State::GoingDown:
                color = CircuitColors::Open; // Light blue
                break;
            case AbstractRelais::State::Down:
            default:
                break;
            }
        }

        painter->setPen(color);

        QRectF textBr;
        drawName(painter, &textBr);

        if(!node()->hideRelayNormalState())
            drawRelayArrow(painter, textRotate(), textBr, int(relayState));
    }
}

QRectF RelaisContactGraphItem::calculateArrowRect(Connector::Direction r, const QRectF& textBr) const
{
    const double halfHeight = 20.0;
    const Connector::Direction arcSide = calculateArcSide();

    QRectF arrowRect;

    Connector::Direction arrowRotate = textRotate();

    if(!node()->hasCentralConnector())
    {
        // Draw in opposite side of name if East/West

        switch (arrowRotate)
        {
        case Connector::Direction::East:
            arrowRotate = Connector::Direction::West;
            break;
        case Connector::Direction::West:
            arrowRotate = Connector::Direction::East;
            break;
        default:
            break;
        }
    }

    const double arrowCenterY = textBr.center().y();
    switch (arrowRotate)
    {
    case Connector::Direction::North:
    {
        arrowRect.setLeft(textBr.right() + 5.0);
        arrowRect.setWidth(21.0);

        arrowRect.setTop(arrowCenterY - halfHeight);
        arrowRect.setBottom(arrowCenterY + halfHeight);
        break;
    }
    case Connector::Direction::South:
    {
        arrowRect.setLeft(textBr.right() + 5.0);
        arrowRect.setWidth(21.0);

        arrowRect.setTop(arrowCenterY - halfHeight);
        arrowRect.setBottom(arrowCenterY + halfHeight);
        break;
    }
    case Connector::Direction::East:
    {
        if(node()->hasCentralConnector())
        {
            arrowRect.setWidth(21.0);
            arrowRect.moveRight(textBr.left() - 2.0);

            arrowRect.setTop(arrowCenterY - halfHeight);
            arrowRect.setBottom(arrowCenterY + halfHeight);
        }
        else
        {
            arrowRect.setLeft(TileLocation::HalfSize + 8.0);
            arrowRect.setRight(TileLocation::Size - 10.0);
            arrowRect.setTop(TileLocation::HalfSize - halfHeight);
            arrowRect.setBottom(TileLocation::HalfSize + halfHeight);

            if(arcSide == arrowRotate)
                arrowRect.moveLeft(arrowRect.left() + arcRadius);
        }

        break;
    }
    case Connector::Direction::West:
    {
        if(node()->hasCentralConnector())
        {
            arrowRect.setWidth(21.0);
            arrowRect.moveLeft(textBr.right() + 2.0);

            arrowRect.setTop(arrowCenterY - halfHeight);
            arrowRect.setBottom(arrowCenterY + halfHeight);
        }
        else
        {
            arrowRect.setLeft(10.0);
            arrowRect.setRight(TileLocation::HalfSize - 8.0);
            arrowRect.setTop(TileLocation::HalfSize - halfHeight);
            arrowRect.setBottom(TileLocation::HalfSize + halfHeight);

            if(arcSide == arrowRotate)
                arrowRect.moveLeft(arrowRect.left() - arcRadius);
        }

        break;
    }
    default:
        break;
    }

    return arrowRect;
}

QRectF RelaisContactGraphItem::boundingRect() const
{
    const double extraMargin = TileLocation::HalfSize;
    QRectF base(-extraMargin, -extraMargin,
                TileLocation::Size + 2 * extraMargin, TileLocation::Size + 2 * extraMargin);

    if(mTextWidth == 0)
        return base;

    // Override to take extra space for relay arrow
    // We cannot override textDisplayRect() because otherwise text is centered
    // also on arrow space.
    const QRectF textRect = textDisplayRect();
    return base.united(textRect)
            .united(calculateArrowRect(textRotate(), textRect));
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
                                            Connector::Direction r,
                                            const QRectF& textBr, int relState)
{
    if(!node()->relais())
        return;

    if(node()->relais()->relaisType() == AbstractRelais::RelaisType::Combinator)
        return; // Combinators do not need arrow

    AbstractRelais::State relayState = AbstractRelais::State(relState);

    if(relayState == AbstractRelais::State::GoingUp
            || relayState == AbstractRelais::State::GoingDown)
        return; // Do not draw arrow for transitory states

    // Draw arrow up/down for normally up/down relays
    const QRectF fullRect = calculateArrowRect(r, textBr);
    const QRectF arrowRect = fullRect.adjusted(0, 3, 0, -3);

    QLineF line;
    QPointF triangle[3];

    const double centerX = arrowRect.center().x();
    const double lineHeight = arrowRect.height() * 0.55;

    const double triangleSemiWidth = 0.5 * qMin(arrowRect.width(),
                                                arrowRect.height() - lineHeight);

    bool isRelayUp = relayState == AbstractRelais::State::Up;
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
    pen.setColor(color);
    pen.setWidthF(2);
    painter->setPen(pen);

    // Draw lines above/below arrow if relay is delayed
    if(node()->relais()->isDelayed(AbstractRelais::State::Up))
    {
        painter->drawLine(centerX - triangleSemiWidth,
                          fullRect.top() + 1,
                          centerX + triangleSemiWidth,
                          fullRect.top() + 1);
    }

    if(node()->relais()->isDelayed(AbstractRelais::State::Down))
    {
        painter->drawLine(centerX - triangleSemiWidth,
                          fullRect.bottom() - 1,
                          centerX + triangleSemiWidth,
                          fullRect.bottom() - 1);
    }

    pen.setWidthF(5);
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

void RelaisContactGraphItem::onRelayCodeChanged()
{

}
