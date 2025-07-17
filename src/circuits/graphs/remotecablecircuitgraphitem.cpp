/**
 * src/circuits/graphs/remotecablecircuitgraphitem.cpp
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

#include "remotecablecircuitgraphitem.h"
#include "../nodes/remotecablecircuitnode.h"

#include "../../objects/circuit_bridge/remotecircuitbridge.h"

#include "../circuitscene.h"
#include "../view/circuitlistmodel.h"

#include "circuitcolors.h"

#include <QPainter>

RemoteCableCircuitGraphItem::RemoteCableCircuitGraphItem(RemoteCableCircuitNode *node_)
    : AbstractNodeGraphItem(node_)
{
    connect(node(), &RemoteCableCircuitNode::modeChanged,
            this, &RemoteCableCircuitGraphItem::triggerUpdate);
}

void RemoteCableCircuitGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    QLineF commonLine;

    const auto cableDirection = toConnectorDirection(rotate());
    switch (cableDirection)
    {
    case Connector::Direction::South:
    case Connector::Direction::North:
        // From center to South
        commonLine.setP1(QPointF(TileLocation::HalfSize,
                                 TileLocation::HalfSize));
        commonLine.setP2(QPointF(TileLocation::HalfSize,
                                 TileLocation::Size));

        // From center to North
        if(cableDirection == Connector::Direction::North)
            commonLine.setP2(QPointF(TileLocation::HalfSize,
                                     0));

        break;

    case Connector::Direction::East:
    case Connector::Direction::West:
        // From center to East
        commonLine.setP1(QPointF(TileLocation::HalfSize,
                                 TileLocation::HalfSize));
        commonLine.setP2(QPointF(TileLocation::Size,
                                 TileLocation::HalfSize));

        // From center to West
        if(cableDirection == Connector::Direction::West)
            commonLine.setP2(QPointF(0, TileLocation::HalfSize));
        break;
    default:
        break;
    }

    const QColor colors[3] =
        {
            CircuitColors::Open,
            CircuitColors::Closed,
            CircuitColors::None
        };

    // Draw wires
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidthF(10.0);
    pen.setCapStyle(Qt::FlatCap);
    pen.setStyle(Qt::DashLine);

    // Draw common line dashed (0)
    pen.setColor(colors[int(node()->hasAnyCircuit(0))]);
    painter->setPen(pen);
    painter->drawLine(commonLine);

    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);

    drawName(painter);
}

void RemoteCableCircuitGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

QString RemoteCableCircuitGraphItem::displayString() const
{
    return node()->getDescription();
}

QString RemoteCableCircuitGraphItem::tooltipString() const
{
    const RemoteCircuitBridge *bridge = node()->remote();
    if(!bridge)
        return tr("No remote bridge set!");

    if(bridge->isRemote())
    {
        QString remoteStr;
        bool isConnected = false;
        if(bridge->getRemoteSession() &&
            (bridge->isRemoteSessionConnected() || !bridge->isSerialDeviceConnected()))
        {
            remoteStr = tr("To session <b>%1</b><br>"
                           "Peer node: <b><i>%2</i></b>")
                            .arg(bridge->remoteSessionName(), bridge->peerNodeName());

            isConnected = bridge->isRemoteSessionConnected();

        }
        else if(bridge->getSerialDevice())
        {
            QString inputId, outputId;
            inputId = outputId = tr("None");

            if(bridge->serialInputId() != 0)
                inputId = QString::number(bridge->serialInputId());
            if(bridge->serialOutputId() != 0)
                outputId = QString::number(bridge->serialOutputId());

            remoteStr = tr("To device <b>%1</b><br>"
                           "Input: %2<br>"
                           "Output: %3<br>")
                            .arg(bridge->getSerialDeviceName(), inputId, outputId);

            isConnected = bridge->isSerialDeviceConnected();
        }

        QString statusStr = QLatin1String("<span style=\"color: %1;\">%2</span>")
                                .arg(isConnected ? QLatin1String("green") : QLatin1String("red"),
                                     isConnected ? tr("Connected") : tr("Disconnected"));
        return tr("Bridge <b>%1</b><br>"
                  "%2<br><br>"
                  "Status: %3")
            .arg(bridge->name(), remoteStr, statusStr);
    }

    RemoteCableCircuitNode *otherNode = bridge->getNode(!node()->isNodeA());
    if(!otherNode)
        return tr("Local Bridge <b>%1</b><br>"
                  "Not connected to other node!")
            .arg(bridge->name());

    QString sceneDescr;

    AbstractNodeGraphItem *otherGraph = nullptr;
    if(circuitScene())
    {
        otherGraph = circuitScene()->circuitsModel()->getGraphForNode(otherNode);
    }

    if(otherGraph && otherGraph->circuitScene())
    {
        CircuitScene *otherScene = otherGraph->circuitScene();
        if(otherScene == circuitScene())
        {
            sceneDescr = tr("To other node in this sheet");
        }
        else
        {
            sceneDescr = tr("To other node in:<br>"
                            "<b>%1<br>"
                            "%2</b>")
                             .arg(otherScene->circuitSheetName(),
                                  otherScene->circuitSheetLongName());
        }
    }
    else
    {
        sceneDescr = tr("Error: could not find peer scene");
    }

    return tr("Local Bridge <b>%1</b><br>"
              "%2")
        .arg(bridge->name(), sceneDescr);
}

QRectF RemoteCableCircuitGraphItem::textDisplayRect() const
{
    // Since cable occupies only half of the tile rect
    // we override to make text nearer to cable by using other half of tile rect.
    QRectF textRect;
    switch (textRotate())
    {
    case Connector::Direction::North:
        textRect.setTop(TileLocation::HalfSize - 2 * TextDisplayMargin - TextDisplayHeight);
        textRect.setBottom(TileLocation::HalfSize - TextDisplayMargin);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);
        break;
    case Connector::Direction::South:
        textRect.setTop(TileLocation::HalfSize + TextDisplayMargin);
        textRect.setBottom(TileLocation::HalfSize + 2 * TextDisplayMargin + TextDisplayHeight);
        textRect.setLeft(-(mTextWidth + 1) / 2 + TileLocation::HalfSize);
        textRect.setRight((mTextWidth + 1) / 2 + TileLocation::HalfSize);
        break;
    case Connector::Direction::East:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::Size);
        textRect.setLeft(TileLocation::HalfSize + TextDisplayMargin);
        textRect.setRight(TileLocation::HalfSize + 2 * TextDisplayMargin + mTextWidth);
        break;
    case Connector::Direction::West:
        textRect.setTop(0);
        textRect.setBottom(TileLocation::Size);
        textRect.setLeft(TileLocation::HalfSize - 2 * TextDisplayMargin - mTextWidth);
        textRect.setRight(TileLocation::HalfSize - TextDisplayMargin);
        break;
    default:
        break;
    }

    return textRect;
}

RemoteCableCircuitNode *RemoteCableCircuitGraphItem::node() const
{
    return static_cast<RemoteCableCircuitNode *>(getAbstractNode());
}
