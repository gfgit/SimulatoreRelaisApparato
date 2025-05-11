/**
 * src/circuits/graphs/buttoncontactgraphitem.cpp
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

#include "buttoncontactgraphitem.h"

#include "../nodes/buttoncontactnode.h"

#include "../../objects/interfaces/buttoninterface.h"
#include "../../objects/button/genericbuttonobject.h"

#include "../../utils/enum_desc.h"

#include <QPainter>

ButtonContactGraphItem::ButtonContactGraphItem(ButtonContactNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

}

void ButtonContactGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    drawDeviator(painter,
                 node()->isContactOn(AbstractDeviatorNode::UpIdx),
                 node()->isContactOn(AbstractDeviatorNode::DownIdx));

    // Draw name
    QColor color = Qt::black;

    if(node()->buttonIface())
    {
        switch (node()->buttonIface()->state())
        {
        case ButtonInterface::State::Pressed:
            color = Qt::darkGreen;
            break;
        case ButtonInterface::State::Extracted:
            color = Qt::red;
            break;
        case ButtonInterface::State::Normal:
        default:
            break;
        }
    }

    painter->setPen(color);

    TileRotate nameRotate = rotate();
    if(node()->flipContact())
        nameRotate += TileRotate::Deg180;

    drawName(painter,
             node()->button() ? node()->button()->name() : tr("NULL"),
             nameRotate);
}

QString ButtonContactGraphItem::displayString() const
{
    if(node()->button())
        return node()->button()->name();
    return QLatin1String("BUT!");
}

QString ButtonContactGraphItem::tooltipString() const
{
    if(!node()->button())
        return tr("No Button set!");

    QString buttonState;
    auto buttonIface = node()->buttonIface();
    if(buttonIface)
    {
        QString butPosName = ButtonInterface::getStateDesc().name(int(buttonIface->state()));
        buttonState = tr("State: <b>%1</b><br>").arg(butPosName);
    }

    return tr("Contact of button <b>%1</b><br>"
              "%2"
              "%3")
            .arg(node()->button()->name(),
                 buttonState,
                 getContactTooltip());
}

ButtonContactNode *ButtonContactGraphItem::node() const
{
    return static_cast<ButtonContactNode *>(getAbstractNode());
}
