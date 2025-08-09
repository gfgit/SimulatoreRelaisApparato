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

    drawScreenRelayPreview(painter);

    // Draw name
    QColor color = Qt::black;
    painter->setPen(color);

    QRectF textBr;
    drawName(painter, &textBr);
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

void ScreenRelaisContactGraphItem::drawScreenRelayPreview(QPainter *painter)
{
    if(!node()->screenRelais())
        return;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(3);
    pen.setColor(Qt::black);

    const QRectF previewRect = itemPreviewRect();
    const double maxWidth = qMin(previewRect.width(), previewRect.height());

    QRectF squarePreview = QRectF(0, 0, maxWidth, maxWidth);
    squarePreview.moveCenter(previewRect.center());


    QRectF circle = QRectF(0, 0, maxWidth * 0.6, maxWidth * 0.6);
    circle.moveCenter(squarePreview.center());
    circle.moveBottom(squarePreview.bottom() - pen.widthF() / 2.0);

    const double outerHeight = 2 * (circle.center().y() - squarePreview.top());
    QRectF circleOuter = QRectF(0, 0, outerHeight, outerHeight);
    circleOuter.moveCenter(circle.center());

    // Draw screen center
    painter->setBrush(Qt::NoBrush);
    painter->setPen(pen);
    painter->drawEllipse(circle);

    // Draw screen
    QPainterPath path;
    path.arcMoveTo(circle, 45);
    path.arcTo(circle, 45, 90);
    path.arcTo(circleOuter, 135, -90);
    path.closeSubpath();
    painter->fillPath(path, Qt::black);

    // Draw Contact name
    QFont f;
    f.setPointSize(circle.height() * 0.7);
    painter->setFont(f);

    const QLatin1String contactName = QLatin1String(node()->isContactA() ? "a" : "b");
    painter->drawText(circle, Qt::AlignCenter, contactName);
}
