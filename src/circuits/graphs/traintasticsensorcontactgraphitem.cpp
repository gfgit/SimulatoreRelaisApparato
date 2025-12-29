/**
 * src/circuits/graphs/traintasticsensorcontactgraphitem.cpp
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

#include "traintasticsensorcontactgraphitem.h"

#include "../nodes/traintasticsensornode.h"

#include "../../objects/traintastic/traintasticsensorobj.h"

#include "../../views/modemanager.h"

#include <QPainter>

TraintasticSensorGraphItem::TraintasticSensorGraphItem(TraintasticSensorNode *node_)
    : AbstractDeviatorGraphItem(node_)
{

}

void TraintasticSensorGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractDeviatorGraphItem::paint(painter, option, widget);

    drawDeviator(painter,
                 node()->isContactOn(AbstractDeviatorNode::UpIdx),
                 node()->isContactOn(AbstractDeviatorNode::DownIdx));

    drawSensorPreview(painter);

    // Draw name
    QColor color = Qt::black;
    painter->setPen(color);

    drawName(painter);
}

QString TraintasticSensorGraphItem::displayString() const
{
    if(node()->sensor())
        return node()->sensor()->name();
    return QLatin1String("SENS!");
}

QString TraintasticSensorGraphItem::tooltipString() const
{
    if(!node()->sensor())
        return tr("No Sensor set!");

    return tr("Contact of sensor <b>%1</b><br>"
              "State: %2")
            .arg(node()->sensor()->name())
            .arg(node()->sensor()->state());
}

TraintasticSensorNode *TraintasticSensorGraphItem::node() const
{
    return static_cast<TraintasticSensorNode *>(getAbstractNode());
}

void TraintasticSensorGraphItem::drawSensorPreview(QPainter *painter)
{
    if(!node()->sensor())
        return;

    const QRectF previewRect = itemPreviewRect();

    // Make square and a bit smaller
    const double maxWidth = qMin(previewRect.width(), previewRect.height()) * 0.9;

    QRectF squarePreview = QRectF(0, 0, maxWidth, maxWidth);
    squarePreview.moveCenter(previewRect.center());

    painter->fillRect(squarePreview, Qt::darkYellow);
}
