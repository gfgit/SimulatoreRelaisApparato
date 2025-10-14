/**
 * src/circuits/graphs/commandnodegraphitem.cpp
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

#include "commandnodegraphitem.h"

#include "../nodes/commandnode.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/simulationobjectfactory.h"

#include "../../views/modemanager.h"

#include "../../utils/enum_desc.h"

#include <QPainter>

CommandNodeGraphItem::CommandNodeGraphItem(CommandNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void CommandNodeGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    // TODO: proper drawing
    painter->fillRect(baseTileRect(), Qt::cyan);

    drawName(painter);
}

void CommandNodeGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

QString CommandNodeGraphItem::displayString() const
{
    if(node()->object())
        return node()->object()->name();
    return QLatin1String("OBJ!");
}

QString CommandNodeGraphItem::tooltipString() const
{
    if(!node()->object())
        return tr("No object set!");

    const QString objType = node()->object()->getType();
    const QString prettyType = node()->modeMgr()->objectFactory()->prettyName(objType);

    QString stateDesc;

    const int targetState = node()->targetPosition();
    EnumDesc desc;
    if(node()->getObjectPosDesc(desc))
    {
        stateDesc = desc.name(targetState);
    }

    if(stateDesc.isEmpty())
        stateDesc = tr("Unknown (%1)").arg(targetState);

    return tr("Command Node:<br>"
              "%1 <b>%2</b><br>"
              "Target State: <b>%3</b>")
            .arg(prettyType, node()->object()->name(),
                 stateDesc);
}

CommandNode *CommandNodeGraphItem::node() const
{
    return static_cast<CommandNode *>(getAbstractNode());
}
