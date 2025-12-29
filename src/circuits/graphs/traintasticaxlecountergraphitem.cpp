/**
 * src/circuits/graphs/traintasticaxlecountergraphitem.cpp
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

#include "traintasticaxlecountergraphitem.h"

#include "../nodes/traintasticaxlecounternode.h"
#include "../circuitscene.h"

#include "../../objects/traintastic/traintasticaxlecounterobj.h"

#include <QPainter>

TraintasticAxleCounterGraphItem::TraintasticAxleCounterGraphItem(TraintasticAxleCounterNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void TraintasticAxleCounterGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    TraintasticAxleCounterObj::State state = TraintasticAxleCounterObj::State::Reset;
    if(node()->axleCounter())
        state = node()->axleCounter()->state();

    QPen borderPen(Qt::black, 3);
    painter->setPen(borderPen);

    if(state == TraintasticAxleCounterObj::State::Reset)
        painter->setBrush(Qt::green);
    else if(state == TraintasticAxleCounterObj::State::ResetPre)
        painter->setBrush(Qt::darkGreen);
    else if(state == TraintasticAxleCounterObj::State::ResetPost)
        painter->setBrush(Qt::darkYellow);
    else
        painter->setBrush(Qt::lightGray);

    const qreal borderW2 = borderPen.widthF() / 2.0;
    const QRectF r = baseTileRect().adjusted(borderW2, borderW2, -borderW2, -borderW2);
    painter->drawRect(r);

    // Draw side letter
    QFont f;
    f.setBold(true);
    f.setPointSizeF(8);
    painter->setFont(f);

    const QChar sideLetters[4] = {'P', 'F', 'R', 'B'};
    const int startIdx = int(rotate());

    painter->setPen(Qt::black);
    QRectF sideLetterRect(QPointF(), QSizeF(12, 12));

    // Up
    sideLetterRect.moveCenter(r.center());
    sideLetterRect.moveTop(r.top());
    painter->drawText(sideLetterRect, Qt::AlignCenter, sideLetters[(2 + startIdx) % 4]);

    // Left
    sideLetterRect.moveCenter(r.center());
    sideLetterRect.moveLeft(r.left());
    painter->drawText(sideLetterRect, Qt::AlignCenter, sideLetters[(3 + startIdx) % 4]);

    // Down
    sideLetterRect.moveCenter(r.center());
    sideLetterRect.moveBottom(r.bottom());
    painter->drawText(sideLetterRect, Qt::AlignCenter, sideLetters[(0 + startIdx) % 4]);

    // Right
    sideLetterRect.moveCenter(r.center());
    sideLetterRect.moveRight(r.right());
    painter->drawText(sideLetterRect, Qt::AlignCenter, sideLetters[(1 + startIdx) % 4]);

    // Now d
    if(state == TraintasticAxleCounterObj::State::Free)
        painter->setPen(Qt::darkGreen);
    else
        painter->setPen(Qt::red);

    f.setPointSize(16);
    painter->setFont(f);

    if(state == TraintasticAxleCounterObj::State::OccupiedAtStart)
        painter->drawText(r, Qt::AlignCenter, tr("Power!"));
    else if(state == TraintasticAxleCounterObj::State::Reset)
        painter->drawText(r, Qt::AlignCenter, tr("Reset!"));
    else if(state == TraintasticAxleCounterObj::State::ResetPre || state == TraintasticAxleCounterObj::State::ResetPost)
        painter->drawText(r, Qt::AlignCenter, tr("WAIT"));
    else
        painter->drawText(r, Qt::AlignCenter, QString::number(node()->axleCounter()->axleCount()));
}

void TraintasticAxleCounterGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), TraintasticAxleCounterNode::Contacts::PowerIn);
    connectors.emplace_back(location(), rotate() - TileRotate::Deg90, TraintasticAxleCounterNode::Contacts::FreeTrackOut);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, TraintasticAxleCounterNode::Contacts::ResetIn);
    connectors.emplace_back(location(), rotate() + TileRotate::Deg90, TraintasticAxleCounterNode::Contacts::OccupiedTrackOut);
}

QString TraintasticAxleCounterGraphItem::displayString() const
{
    if(node()->axleCounter())
        return node()->axleCounter()->name();
    return QLatin1String("AXLCNT!");
}

QString TraintasticAxleCounterGraphItem::tooltipString() const
{
    if(!node()->axleCounter())
        return tr("No Axle Counter set!");

    return tr("Traintastic Axle Counter <b>%1</b><br>"
              "State: <b>%2</b><br>"
              "Axle cout: <b>%3</b>")
            .arg(node()->axleCounter()->name(), node()->axleCounter()->getStateName())
            .arg(node()->axleCounter()->axleCount());
}

TraintasticAxleCounterNode *TraintasticAxleCounterGraphItem::node() const
{
    return static_cast<TraintasticAxleCounterNode *>(getAbstractNode());
}
