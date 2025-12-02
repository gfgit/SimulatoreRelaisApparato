/**
 * src/circuits/graphs/traintasticturnoutgraphitem.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#include "traintasticturnoutgraphitem.h"

#include "../nodes/traintasticturnoutnode.h"

#include "../../objects/traintastic/traintasticturnoutobj.h"
#include "../../objects/traintastic/traintasticspawnobj.h"
#include "../../objects/traintastic/traintasticauxsignalobject.h"

#include <QPainter>

TraintasticTurnoutGraphItem::TraintasticTurnoutGraphItem(TraintasticTurnoutNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void TraintasticTurnoutGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    QRectF rectN, rectR;

    const auto cableDirection = toConnectorDirection(rotate());
    switch (cableDirection)
    {
    case Connector::Direction::South:
    case Connector::Direction::North:
        rectN.setTop(TileLocation::HalfSize);
        rectN.setBottom(TileLocation::Size);
        rectN.setLeft(TileLocation::Size / 3);
        rectN.setRight(TileLocation::Size / 3 * 2);

        rectR.setTop(0);
        rectR.setBottom(TileLocation::HalfSize);
        rectR.setLeft(TileLocation::Size / 3);
        rectR.setRight(TileLocation::Size / 3 * 2);

        if(cableDirection == Connector::Direction::North)
        {
            std::swap(rectN, rectR);
        }
        break;

    case Connector::Direction::East:
    case Connector::Direction::West:
        rectN.setLeft(0);
        rectN.setRight(TileLocation::HalfSize);
        rectN.setTop(TileLocation::Size / 3);
        rectN.setBottom(TileLocation::Size / 3 * 2);

        rectR.setLeft(TileLocation::HalfSize);
        rectR.setRight(TileLocation::Size);
        rectR.setTop(TileLocation::Size / 3);
        rectR.setBottom(TileLocation::Size / 3 * 2);

        if(cableDirection == Connector::Direction::West)
        {
            std::swap(rectN, rectR);
        }
        break;
    default:
        break;
    }

    // Motor circle
    painter->setPen(QPen(Qt::black, 3));
    painter->setBrush(node()->spawn() ? Qt::darkCyan : Qt::lightGray);
    painter->drawEllipse(baseTileRect());

    if(!node()->spawn())
    {
        // Letters
        painter->fillRect(rectN, Qt::gray);
        painter->fillRect(rectR, Qt::gray);

        QFont f;
        f.setPointSize(std::min(rectN.height(), rectN.width()) * 0.5);
        painter->setFont(f);

        painter->setBrush(Qt::NoBrush);
        painter->drawText(rectN, "N", QTextOption(Qt::AlignCenter));
        painter->drawText(rectR, "R", QTextOption(Qt::AlignCenter));
    }

    drawName(painter);
}

void TraintasticTurnoutGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    if(!node()->spawn())
        connectors.emplace_back(location(), rotate(), 1);

    connectors.emplace_back(location(), rotate() + TileRotate::Deg180, 0);
}

QString TraintasticTurnoutGraphItem::displayString() const
{
    if(node()->turnout())
        return node()->turnout()->name();
    else if(node()->spawn())
        return node()->spawn()->name();
    else if(node()->auxSignal())
        return node()->auxSignal()->name();
    else
        return tr("DEV!");
}

QString TraintasticTurnoutGraphItem::tooltipString() const
{
    if(node()->turnout())
        return tr("Traintastic Turnout:<br>"
                  "<b>%1</b>").arg(node()->turnout()->name());
    else if(node()->spawn())
        return tr("Traintastic Spawn:<br>"
                  "<b>%1</b>").arg(node()->spawn()->name());
    else if(node()->auxSignal())
        return tr("Traintastic Aux Signal:<br>"
                  "<b>%1</b>").arg(node()->auxSignal()->name());
    else
        return tr("No Traintastic turnout, spawn or aux signal set!");

}

TraintasticTurnoutNode *TraintasticTurnoutGraphItem::node() const
{
    return static_cast<TraintasticTurnoutNode *>(getAbstractNode());
}
