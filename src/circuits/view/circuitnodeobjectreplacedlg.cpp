/**
 * src/circuits/view/circuitnodeobjectreplacedlg.cpp
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

#include "circuitnodeobjectreplacedlg.h"

#include "../graphs/abstractnodegraphitem.h"
#include "../nodes/abstractcircuitnode.h"

#include "../../views/modemanager.h"
#include "../../views/viewmanager.h"

#include "../edit/nodeeditfactory.h"

CircuitNodeObjectReplaceDlg::CircuitNodeObjectReplaceDlg(ViewManager *viewMgr,
                                                         const QVector<AbstractNodeGraphItem *> &items,
                                                         QWidget *parent)
    : ItemObjectReplaceDlg<CircuitNodeTraits>(viewMgr, items, parent)
{

}

void CircuitNodeTraits::getObjectProperties(Node *node, QVector<ObjectProperty> &result)
{
    return node->getAbstractNode()->getObjectProperties(result);
}

QString CircuitNodeTraits::getNodeType(Node *node)
{
    return node->getAbstractNode()->nodeType();
}

bool CircuitNodeTraits::loadFromJSON(Node *node, const QJsonObject &obj)
{
    return node->loadFromJSON(obj);
}

void CircuitNodeTraits::saveToJSON(Node *node, QJsonObject &obj)
{
    node->saveToJSON(obj);
}

void CircuitNodeTraits::editItem(Node *node, ViewManager *viewMgr, QWidget *parent)
{
    auto editFactory = viewMgr->modeMgr()->circuitFactory();
    editFactory->editItem(parent, node, viewMgr, false);
}

QStringList CircuitNodeTraits::getRegisteredTypes(ModeManager *modeMgr)
{
    auto editFactory = modeMgr->circuitFactory();
    return editFactory->getRegisteredTypes();
}

QString CircuitNodeTraits::prettyTypeName(ModeManager *modeMgr, const QString &typeName)
{
    auto editFactory = modeMgr->circuitFactory();
    return editFactory->prettyName(typeName);
}

#include "../../utils/itemobjectreplacedlg_impl.hpp"
