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

#include <QPainter>

ElectroMagnetContactGraphItem::ElectroMagnetContactGraphItem(ElectromagnetContactNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

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

    drawName(painter);
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

