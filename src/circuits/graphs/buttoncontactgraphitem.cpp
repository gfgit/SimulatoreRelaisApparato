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

#include "../../views/modemanager.h"

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

    drawButtonPreview(painter);

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

    drawName(painter);
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

void ButtonContactGraphItem::drawButtonPreview(QPainter *painter)
{
    if(!node()->buttonIface())
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


    QRectF circle = QRectF(0, 0, maxWidth * 0.7, maxWidth * 0.7);
    circle.moveCenter(squarePreview.center());

    QRectF circleOuter = squarePreview.adjusted(-pen.widthF() / 2.0, -pen.widthF() / 2.0,
                                                pen.widthF() / 2.0, pen.widthF() / 2.0);

    QPainterPath extractedPath;
    extractedPath.addEllipse(circleOuter);
    extractedPath.addEllipse(circle);

    ButtonInterface::State butState = node()->buttonIface()->state();

    if(node()->modeMgr()->mode() != FileMode::Simulation)
    {
        butState = ButtonInterface::State::Normal;

        bool normallyOff = !node()->getContactStateFor(int(ButtonInterface::State::Normal), 0);
        bool onlyPressed = false;
        bool onlyExtracted = false;
        if(normallyOff)
        {
            if(node()->getContactStateFor(int(ButtonInterface::State::Pressed), 0))
            {
                onlyPressed = true;
            }

            if(node()->getContactStateFor(int(ButtonInterface::State::Extracted), 0))
            {
                onlyExtracted = true;
            }
        }

        if(node()->hasCentralConnector())
        {
            if(normallyOff)
                normallyOff = !node()->getContactStateFor(int(ButtonInterface::State::Normal), 1);

            if(normallyOff)
            {
                if(node()->getContactStateFor(int(ButtonInterface::State::Pressed), 1))
                {
                    onlyPressed = true;
                }

                if(node()->getContactStateFor(int(ButtonInterface::State::Extracted), 1))
                {
                    onlyExtracted = true;
                }
            }
        }

        if(!normallyOff || (onlyPressed && onlyExtracted))
            butState = ButtonInterface::State::Normal;
        else if(onlyPressed)
            butState = ButtonInterface::State::Pressed;
        else if(onlyExtracted)
            butState = ButtonInterface::State::Extracted;
    }

    painter->setPen(pen);

    if(butState == ButtonInterface::State::Extracted)
        painter->setBrush(Qt::black); // Fill outer circle ring

    painter->drawPath(extractedPath);

    if(butState == ButtonInterface::State::Pressed)
    {
        // Fill inner circle
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);

        painter->drawEllipse(circle);
    }
}
