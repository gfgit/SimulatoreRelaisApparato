/**
 * src/circuits/graphs/simpleactivationgraphitem.cpp
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

#include "simpleactivationgraphitem.h"

#include "../nodes/simpleactivationnode.h"

#include "../../objects/simple_activable/abstractsimpleactivableobject.h"

#include "../../views/modemanager.h"
#include "../../objects/simulationobjectfactory.h"

SimpleActivationGraphItem::SimpleActivationGraphItem(SimpleActivationNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void SimpleActivationGraphItem::getConnectors(std::vector<Connector> &connectors) const
{
    connectors.emplace_back(location(), rotate(), 0);
}

QString SimpleActivationGraphItem::displayString() const
{
    if(activationNode()->object())
        return activationNode()->object()->name();
    return QLatin1String("OBJ!");
}

QString SimpleActivationGraphItem::tooltipString() const
{
    if(!activationNode()->object())
        return tr("No object set!");

    const QString objType = activationNode()->object()->getType();
    const QString prettyType = activationNode()->modeMgr()->objectFactory()->prettyName(objType);

    const auto state_ = activationNode()->object()->state();

    return tr("%1 <b>%2</b><br>"
              "State: <b>%3</b>")
            .arg(prettyType, activationNode()->object()->name(),
                 state_ == AbstractSimpleActivableObject::State::On ? tr("On") : tr("Off"));
}

SimpleActivationNode *SimpleActivationGraphItem::activationNode() const
{
    return static_cast<SimpleActivationNode *>(getAbstractNode());
}
