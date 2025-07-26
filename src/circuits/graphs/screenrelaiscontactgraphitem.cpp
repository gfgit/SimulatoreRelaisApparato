/**
 * src/circuits/graphs/screenrelaiscontactgraphitem.cpp
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

#include "screenrelaiscontactgraphitem.h"

#include "../nodes/screenrelaiscontactnode.h"
#include "../../objects/screen_relais/model/screenrelais.h"

#include "../../views/modemanager.h"

#include "../../utils/enum_desc.h"

#include <QPainter>

ScreenRelaisContactGraphItem::ScreenRelaisContactGraphItem(ScreenRelaisContactNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

}

QRectF ScreenRelaisContactGraphItem::boundingRect() const
{
    const double extraMargin = TileLocation::HalfSize;
    QRectF base(-extraMargin, -extraMargin,
                TileLocation::Size + 2 * extraMargin, TileLocation::Size + 2 * extraMargin);

    if(mTextWidth == 0)
        return base;

    return base.united(textDisplayRect()).united(calculateContactNameRect());
}

void ScreenRelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    bool contactUpOn = node()->isContactOn(AbstractDeviatorNode::UpIdx);
    bool contactDownOn = node()->isContactOn(AbstractDeviatorNode::DownIdx);

    drawDeviator(painter, contactUpOn, contactDownOn);

    // Draw name
    QColor color = Qt::black;
    painter->setPen(color);

    QRectF textBr;
    drawName(painter, &textBr);

    // Draw Contact name
    const QRectF contactNameRect = calculateContactNameRect();

    painter->setPen(Qt::darkGreen);

    const QLatin1String contactName = QLatin1String(node()->isContactA() ? "a" : "b");
    painter->drawText(contactNameRect, Qt::AlignCenter, contactName);
}

QString ScreenRelaisContactGraphItem::displayString() const
{
    if(node()->screenRelais())
        return node()->screenRelais()->name();
    return QLatin1String("SCR!");
}

QString ScreenRelaisContactGraphItem::tooltipString() const
{
    if(!node()->screenRelais())
        return tr("No Screen Relay set!");

    QString typeStr = ScreenRelais::getTypeDesc().name(int(node()->screenRelais()->screenType()));

    return tr("Contact of Screen Relay <b>%1</b><br>"
              "Type: %2<br>"
              "%3")
            .arg(node()->screenRelais()->name(),
                 typeStr,
                 getContactTooltip());
}

QRectF ScreenRelaisContactGraphItem::textDisplayRect() const
{
    // Put text higher in East/West so we can draw cotact name below
    const Connector::Direction arcSide = calculateArcSide();

    const double textDisplayHeight = textDisplayFontSize() * 1.5;
    QRectF textRect;
    switch (textRotate())
    {
    case Connector::Direction::North:
        textRect.setTop(- TextDisplayMarginSmall - textDisplayHeight);
        textRect.setBottom(0);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);

        if(arcSide != textRotate())
            textRect.moveTop(textRect.top() + TileLocation::HalfSize / 2.0);

        break;
    case Connector::Direction::South:
        textRect.setTop(TileLocation::Size);
        textRect.setBottom(TileLocation::Size + TextDisplayMarginSmall + textDisplayHeight);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);

        if(arcSide != textRotate())
            textRect.moveTop(textRect.top() - TileLocation::HalfSize / 2.0);
        break;
    case Connector::Direction::East:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::HalfSize);
        textRect.setLeft(TileLocation::Size);
        textRect.setRight(TileLocation::Size + TextDisplayMarginSmall + mTextWidth);

        if(arcSide != textRotate())
            textRect.moveLeft(textRect.left() - TileLocation::HalfSize / 2.0);

        if(deviatorNode()->hasCentralConnector())
            textRect.moveLeft(textRect.left() + 2);
        break;
    case Connector::Direction::West:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::HalfSize);
        textRect.setLeft(- TextDisplayMarginSmall - mTextWidth);
        textRect.setRight(0);

        if(arcSide != textRotate())
            textRect.moveLeft(textRect.left() + TileLocation::HalfSize / 2.0);

        if(deviatorNode()->hasCentralConnector())
            textRect.moveLeft(textRect.left() - 2);
        break;
    default:
        break;
    }

    return textRect;
}

ScreenRelaisContactNode *ScreenRelaisContactGraphItem::node() const
{
    return static_cast<ScreenRelaisContactNode *>(getAbstractNode());
}

QRectF ScreenRelaisContactGraphItem::calculateContactNameRect() const
{
    const double halfNameWidth = 12.0;
    const QRectF textBr = textDisplayRect();
    QRectF contactNameRect;

    const QPointF textCenter = textBr.center();

    // Follow drawName() layout to place contact name next to relais name
    switch (textRotate())
    {
    case Connector::Direction::North:
        contactNameRect.setWidth(20.0);
        contactNameRect.moveLeft(textBr.right() + 5.0);
        contactNameRect.setTop(textCenter.y() - halfNameWidth);
        contactNameRect.setBottom(textCenter.y() + halfNameWidth);
        break;

    case Connector::Direction::South:
        contactNameRect.setWidth(20.0);
        contactNameRect.moveLeft(textBr.right() + 5.0);
        contactNameRect.setTop(textCenter.y() - halfNameWidth);
        contactNameRect.setBottom(textCenter.y() + halfNameWidth);
        break;

    case Connector::Direction::East:
        contactNameRect.setLeft(textCenter.x() - halfNameWidth);
        contactNameRect.setRight(textCenter.x() + halfNameWidth);
        contactNameRect.setTop(TileLocation::HalfSize + 2.0);
        contactNameRect.setBottom(TileLocation::Size - 20.0);
        break;

    case Connector::Direction::West:
        contactNameRect.setLeft(textCenter.x() - halfNameWidth);
        contactNameRect.setRight(textCenter.x() + halfNameWidth);
        contactNameRect.setTop(TileLocation::HalfSize + 2.0);
        contactNameRect.setBottom(TileLocation::Size - 20.0);
        break;

    default:
        break;
    }

    return contactNameRect;
}
