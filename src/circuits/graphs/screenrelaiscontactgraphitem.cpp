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

void ScreenRelaisContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    bool contactUpOn = node()->isContactOn(AbstractDeviatorNode::UpIdx);
    bool contactDownOn = node()->isContactOn(AbstractDeviatorNode::DownIdx);

    drawDeviator(painter, contactUpOn, contactDownOn);

    // Draw name
    QColor color = Qt::black;
    painter->setPen(color);

    TileRotate nameRotate = rotate();
    if(node()->flipContact())
        nameRotate += TileRotate::Deg180;

    QRectF textBr;
    drawName(painter,
             node()->screenRelais() ? node()->screenRelais()->name() : tr("REL!"),
             nameRotate, &textBr);

    // Draw Contact name
    QRectF contactNameRect;

    // Follow drawName() layout to place contact name next to relais name
    switch (toConnectorDirection(rotate() - TileRotate::Deg90))
    {
    case Connector::Direction::North:
        contactNameRect.setLeft(textBr.right() + 5.0);
        contactNameRect.setWidth(10.0);
        contactNameRect.setTop(10.0);
        contactNameRect.setBottom(TileLocation::HalfSize - 12.0);
        break;

    case Connector::Direction::South:
        contactNameRect.setLeft(textBr.right() + 5.0);
        contactNameRect.setWidth(10.0);
        contactNameRect.setTop(TileLocation::HalfSize + 12.0);
        contactNameRect.setBottom(TileLocation::Size - 10.0);
        break;

    case Connector::Direction::East:
        contactNameRect.setLeft(TileLocation::HalfSize + 3.0);
        contactNameRect.setRight(TileLocation::Size - 10.0);
        contactNameRect.setTop(TileLocation::HalfSize + 2.0);
        contactNameRect.setBottom(TileLocation::Size - 20.0);
        break;

    case Connector::Direction::West:
        contactNameRect.setLeft(10.0);
        contactNameRect.setRight(TileLocation::HalfSize - 3.0);
        contactNameRect.setTop(TileLocation::HalfSize + 2.0);
        contactNameRect.setBottom(TileLocation::Size - 20.0);
        break;

    default:
        break;
    }

    // Don't draw out of item's tile rect
    if(contactNameRect.right() > TileLocation::Size)
        contactNameRect.setRight(TileLocation::Size);

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

ScreenRelaisContactNode *ScreenRelaisContactGraphItem::node() const
{
    return static_cast<ScreenRelaisContactNode *>(getAbstractNode());
}
