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
}

void RelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    bool contactUpOn = node()->isContactOn(AbstractDeviatorNode::UpIdx);
    bool contactDownOn = node()->isContactOn(AbstractDeviatorNode::DownIdx);

    AbstractRelais::State relayState = AbstractRelais::State::Down;

    bool fillArc = true;

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

        switch (node()->relais()->relaisType())
        {
        case AbstractRelais::RelaisType::Encoder:
        case AbstractRelais::RelaisType::CodeRepeater:
        {
            // Omit arc fill for theese types
            fillArc = false;
            break;
        }
        default:
        {
            AbstractRelais::State defState = AbstractRelais::State::Up;
            if(!node()->relais()->normallyUp())
                defState = AbstractRelais::State::Down;

            // Fill arc when not in normal state
            fillArc = (node()->relais()->state() != defState);
            break;
        }
        }
    }

    drawDeviator(painter, contactUpOn, contactDownOn, fillArc);

    // Draw name
    QColor color = Qt::black;
    if(node()->relais())
    {
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
    }

    painter->setPen(color);
    QRectF textBr;
    drawName(painter, &textBr);

    drawRelayPreview(painter);

    if(!node()->hideRelayNormalState())
        drawRelayArrow(painter, textRotate(), textBr, int(relayState));
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
            .united(calculateArrowRect(textRotate(), textRect))
            .united(itemPreviewRect());
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

void RelaisContactGraphItem::drawRelayPreview(QPainter *painter)
{
    if(!node()->relais())
        return;

    const QRectF previewRect = itemPreviewRect();

    QColor color = CircuitColors::None;
    switch (node()->relais()->state())
    {
    case AbstractRelais::State::Up:
        color = CircuitColors::Closed;
        break;
    case AbstractRelais::State::GoingDown:
    case AbstractRelais::State::GoingUp:
        color = CircuitColors::Open;
        break;
    default:
        break;
    }

    QPen pen;
    pen.setWidthF(5.0);
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);

    // Half rect - half pen width
    const double relayRadius = (qMin(previewRect.height(), previewRect.width()) - pen.widthF()) / 2.0;

    QRectF relayRect = QRectF(0, 0, relayRadius * 2, relayRadius * 2);
    relayRect.moveCenter(previewRect.center());

    const QRectF lineRect = relayRect.adjusted(-pen.widthF() / 2.0, 0,
                                               +pen.widthF() / 2.0, 0);

    switch (node()->relais()->relaisType())
    {
    case AbstractRelais::RelaisType::Polarized:
    case AbstractRelais::RelaisType::PolarizedInverted:
    {
        // Draw a diode symbol, a bit bigger because we skip relay circle
        const double relCenterX = relayRect.center().x();
        const double relCenterY = relayRect.center().y();
        const double halfHeight = relayRadius * 0.7;
        QPointF triangle[3] =
        {
            {relCenterX - halfHeight * 0.86, relCenterY},
            {relCenterX + halfHeight, relCenterY - halfHeight},
            {relCenterX + halfHeight, relCenterY + halfHeight}
        };

        if(node()->relais()->relaisType() == AbstractRelais::RelaisType::Polarized)
        {
            // Invert diode
            std::swap(triangle[0].rx(), triangle[1].rx());
            triangle[2].rx() = triangle[1].x();
        }

        // Diode triangle
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawPolygon(triangle, 3);

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        // Diode vertical line
        painter->drawLine(QLineF(triangle[0].x(), triangle[1].y(),
                                 triangle[0].x(), triangle[2].y()));
        break;
    }
    case AbstractRelais::RelaisType::Stabilized:
    {
        // Stabilized relais have a slice
        // The slice represent the permanent magnet
        // We use it instead to show disabling coil,
        // which if active brings the relay down.
        // For relay contact we always draw it inactive
        const int upCoilAngleStart = -90 * 16;

        // Draw slice on second connector side (black)
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        painter->drawPie(relayRect,
                         upCoilAngleStart - 180 * 16,
                         180 * 16);
        break;
    }
    case AbstractRelais::RelaisType::Combinator:
    {
        // Use smaller pen
        pen.setWidthF(4.0);

        // Always draw first coil
        relayRect.setLeft(previewRect.left() + pen.widthF() / 2.0);
        relayRect.setWidth(relayRadius * 0.95);
        relayRect.setHeight(relayRect.width());
        relayRect.moveTop(previewRect.center().y() - relayRect.height() / 2.0);

        // Draw full X near to relay circle
        QPointF startPt(0.725, 0.0534857225127);
        QPointF targetPt(2.1666666666667, 0.7799462554779);

        QPen xPen = pen;
        xPen.setColor(Qt::black);

        painter->setPen(xPen);
        painter->setBrush(Qt::NoBrush);

        painter->drawLine(startPt * relayRect.width() + relayRect.topLeft(),
                          targetPt * relayRect.width() + relayRect.topLeft());

        startPt.ry()  = 1 - startPt.y();
        targetPt.ry() = 1 - targetPt.y();
        painter->drawLine(startPt * relayRect.width() + relayRect.topLeft(),
                          targetPt * relayRect.width() + relayRect.topLeft());
        break;
    }
    case AbstractRelais::RelaisType::Timer:
    {
        // Draw arcs on relay circle
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);

        painter->drawArc(relayRect.translated(-relayRadius, 0), -90 * 16, 180 * 16);
        painter->drawArc(relayRect.translated(relayRadius, 0), -90 * 16, -180 * 16);
        break;
    }
    case AbstractRelais::RelaisType::Blinker:
    case AbstractRelais::RelaisType::Encoder:
    {
        // Draw pie sector 45 degrees
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);

        // Left
        painter->drawPie(relayRect,
                         -45 * 16, 90 * 16);

        painter->drawPie(relayRect,
                         135 * 16, 90 * 16);
        break;
    }
    case AbstractRelais::RelaisType::CodeRepeater:
    {
        // Draw horizontal center line
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);

        const double centerY = lineRect.center().y();
        painter->drawLine(QLineF(lineRect.left(),
                                 centerY,
                                 lineRect.right(),
                                 centerY));
        break;
    }
    case AbstractRelais::RelaisType::DiskRelayAC:
    {
        // Draw a cross onto the circle
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);

        // Adjust rect so that cross lines stay inside the circle
        const double adjLine = relayRadius * 0.27;
        const QRectF circleInner = relayRect.adjusted(adjLine, adjLine,
                                                      -adjLine, -adjLine);

        painter->drawLine(circleInner.topLeft(), circleInner.bottomRight());
        painter->drawLine(circleInner.topRight(), circleInner.bottomLeft());
        break;
    }
    default:
        break;
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    switch (node()->relais()->relaisType())
    {
    case AbstractRelais::RelaisType::Stabilized:
    case AbstractRelais::RelaisType::Timer:
    case AbstractRelais::RelaisType::Blinker:
    case AbstractRelais::RelaisType::Encoder:
    case AbstractRelais::RelaisType::CodeRepeater:
    case AbstractRelais::RelaisType::DiskRelayAC:
    {
        // Draw relay circle
        painter->drawEllipse(relayRect);

        // Draw lines top/bottom
        painter->drawLine(lineRect.topLeft(), lineRect.topRight());
        painter->drawLine(lineRect.bottomLeft(), lineRect.bottomRight());
        break;
    }
    case AbstractRelais::RelaisType::Combinator:
    {
        // Combinator relays have circle but not lines
        painter->drawEllipse(relayRect);
        break;
    }
    case AbstractRelais::RelaisType::Polarized:
    case AbstractRelais::RelaisType::PolarizedInverted:
    {
        // We skip relay circle for polarized relays
        break;
    }
    case AbstractRelais::RelaisType::Normal:
    case AbstractRelais::RelaisType::Decoder:
    case AbstractRelais::RelaisType::NTypes:
    {
        // Other relay types do not have preview
        break;
    }
    }
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

    const double centerX = arrowRect.center().x();
    const double lineHeight = arrowRect.height() * 0.55;

    const double triangleSemiWidth = 0.5 * qMin(arrowRect.width(),
                                                arrowRect.height() - lineHeight);

    bool isRelayUp = relayState == AbstractRelais::State::Up;

    const bool isNotSimulating = node()->modeMgr()->mode() != FileMode::Simulation;
    bool isEncoderOrRepeater = (node()->relais()->relaisType() == AbstractRelais::RelaisType::Encoder
                                || node()->relais()->relaisType() == AbstractRelais::RelaisType::CodeRepeater);

    if(isNotSimulating)
    {
        // In static or editing mode,
        // draw relay in its normal state
        isRelayUp = node()->relais()->normallyUp();
    }

    const QPointF arrowUpTriangle[3] =
    {
        QPointF(centerX, arrowRect.top()),
        QPointF(centerX + triangleSemiWidth, arrowRect.bottom() - lineHeight),
        QPointF(centerX - triangleSemiWidth, arrowRect.bottom() - lineHeight)
    };

    const QPointF arrowDownTriangle[3] =
    {
        QPointF(centerX, arrowRect.bottom()),
        QPointF(centerX + triangleSemiWidth, arrowRect.top() + lineHeight),
        QPointF(centerX - triangleSemiWidth, arrowRect.top() + lineHeight)
    };

    if(isRelayUp)
    {
        // Arrow up
        line.setP1(QPointF(centerX, arrowRect.bottom() - lineHeight));
        line.setP2(QPointF(centerX, arrowRect.bottom()));
    }
    else
    {
        // Arrow down
        line.setP1(QPointF(centerX, arrowRect.top() + lineHeight));
        line.setP2(QPointF(centerX, arrowRect.top()));

        if(isEncoderOrRepeater && isNotSimulating)
        {
            line.setP2(QPointF(centerX, arrowRect.bottom() - lineHeight));
        }
    }

    /* Colors:
     * Black: relay is normally down and currently down
     * Red: relay is normally up and currently up
     *
     * Blue: relay is normally up BUT currently down
     * Purple: relay is normally down BUT currently up
     */

    QColor color = Qt::black;
    if(isEncoderOrRepeater && isNotSimulating)
    {
        color = Qt::blue;
    }
    else if(isRelayUp)
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
    painter->drawPolygon(isRelayUp ? arrowUpTriangle : arrowDownTriangle, 3);

    if(isEncoderOrRepeater && isNotSimulating)
    {
        // Draw also other arrow
        painter->drawPolygon(!isRelayUp ? arrowUpTriangle : arrowDownTriangle, 3);
    }
}

RelaisContactNode *RelaisContactGraphItem::node() const
{
    return static_cast<RelaisContactNode *>(getAbstractNode());
}

QRectF RelaisContactGraphItem::itemPreviewRect() const
{
    if(!node()->relais())
        return QRectF(); // No preview

    switch (node()->relais()->relaisType())
    {
    case AbstractRelais::RelaisType::Normal:
    case AbstractRelais::RelaisType::Decoder:
    {
        // No preview for these types
        return QRectF();
    }
    default:
        break;
    }

    return AbstractDeviatorGraphItem::itemPreviewRect();
}
