/**
 * src/circuits/graphs/electromagnetcontactgraphitem.cpp
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

#include "electromagnetcontactgraphitem.h"

#include "../nodes/electromagnetcontactnode.h"

#include "../../objects/simple_activable/electromagnetobject.h"

#include "../../views/modemanager.h"

#include "../../utils/enum_desc.h"

#include "circuitcolors.h"

#include <QPainter>

ElectroMagnetContactGraphItem::ElectroMagnetContactGraphItem(ElectromagnetContactNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

}

QRectF ElectroMagnetContactGraphItem::boundingRect() const
{
    const double extraMargin = TileLocation::HalfSize;
    QRectF base(-extraMargin, -extraMargin,
                TileLocation::Size + 2 * extraMargin, TileLocation::Size + 2 * extraMargin);

    if(mTextWidth == 0)
        return base;

    // Override to take extra space for electromagnet arrow
    // We cannot override textDisplayRect() because otherwise text is centered
    // also on arrow space.
    const QRectF textRect = textDisplayRect();
    return base.united(textRect)
            .united(calculateArrowRect(textRect))
            .united(itemPreviewRect());
}

void ElectroMagnetContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    drawDeviator(painter,
                 node()->isContactOn(AbstractDeviatorNode::UpIdx),
                 node()->isContactOn(AbstractDeviatorNode::DownIdx));

    drawMagnetPreview(painter);

    // Draw name
    QColor color = Qt::black;

    if(node()->magnet())
    {
        switch (node()->magnet()->state())
        {
        case ElectroMagnetObject::State::On:
            color = Qt::red;
            break;
        default:
            break;
        }
    }

    painter->setPen(color);

    QRectF textBr;
    drawName(painter, &textBr);

    drawElectroMagnetArrow(painter, textBr);
}

QString ElectroMagnetContactGraphItem::displayString() const
{
    if(node()->magnet())
        return node()->magnet()->name();
    return QLatin1String("MAGN!");
}

QString ElectroMagnetContactGraphItem::tooltipString() const
{
    if(!node()->magnet())
        return tr("No Magnet set!");

    QString magnetState = tr("State: <b>%1</b><br>")
            .arg(node()->magnet()->state() == ElectroMagnetObject::State::On ?
                     tr("On") : tr("Off"));

    return tr("Contact of magnet <b>%1</b><br>"
              "%2"
              "%3")
            .arg(node()->magnet()->name(),
                 magnetState,
                 getContactTooltip());
}

ElectromagnetContactNode *ElectroMagnetContactGraphItem::node() const
{
    return static_cast<ElectromagnetContactNode *>(getAbstractNode());
}

void ElectroMagnetContactGraphItem::drawMagnetPreview(QPainter *painter)
{
    if(!node()->magnet())
        return;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(3);
    pen.setColor(Qt::black);

    const QRectF previewRect = itemPreviewRect();

    // Make square and a bit smaller
    const double maxWidth = qMin(previewRect.width(), previewRect.height()) * 0.9;

    QRectF squarePreview = QRectF(0, 0, maxWidth, maxWidth);
    squarePreview.moveCenter(previewRect.center());


    QRectF circle = QRectF(0, 0, maxWidth, maxWidth);
    circle.moveCenter(squarePreview.center());

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);

    painter->drawEllipse(circle);
}

QRectF ElectroMagnetContactGraphItem::calculateArrowRect(const QRectF& textBr) const
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

void ElectroMagnetContactGraphItem::drawElectroMagnetArrow(QPainter *painter,
                                                           const QRectF& textBr)
{
    if(!node()->magnet())
        return;

    // Draw arrow up/down for normally up/down relays
    const QRectF fullRect = calculateArrowRect(textBr);
    const QRectF arrowRect = fullRect.adjusted(0, 3, 0, -3);

    QLineF line;

    const double centerX = arrowRect.center().x();
    const double lineHeight = arrowRect.height() * 0.55;

    const double triangleSemiWidth = 0.5 * qMin(arrowRect.width(),
                                                arrowRect.height() - lineHeight);

    const auto curState = node()->magnet()->state();
    const auto electricalState = node()->magnet()->electricalState();

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

    if(curState == AbstractSimpleActivableObject::State::On)
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
    }

    /* Colors:
     * Black: relay is normally down and currently down
     * Red: relay is normally up and currently up
     *
     * Blue: relay is normally up BUT currently down
     * Purple: relay is normally down BUT currently up
     */

    QColor color = Qt::black;

    if(curState == electricalState)
    {
        if(curState == AbstractSimpleActivableObject::State::On)
            color = CircuitColors::Closed;
    }
    else if(curState == AbstractSimpleActivableObject::State::On)
        color = qRgb(242, 157, 0); // Forced up (Orange)
    else if(electricalState == AbstractSimpleActivableObject::State::On)
        color = Qt::blue; // Forced down

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);
    pen.setWidthF(2);
    painter->setPen(pen);

    pen.setWidthF(5);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(line);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(curState == AbstractSimpleActivableObject::State::On ?
                             arrowUpTriangle :
                             arrowDownTriangle, 3);
}

