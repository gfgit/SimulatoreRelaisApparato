/**
 * src/panels/view/panelitemobjectreplacedlg.cpp
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

#include "panelitemobjectreplacedlg.h"

#include "../abstractpanelitem.h"

#include "../../views/modemanager.h"
#include "../../views/viewmanager.h"

#include "../edit/panelitemfactory.h"

PanelItemObjectReplaceDlg::PanelItemObjectReplaceDlg(ViewManager *viewMgr,
                                                     const QVector<AbstractPanelItem *> &items,
                                                     QWidget *parent)
    : ItemObjectReplaceDlg<PanelItemTraits>(viewMgr, items, parent)
{

}

void PanelItemTraits::getObjectProperties(Node *node, QVector<ObjectProperty> &result)
{
    return node->getObjectProperties(result);
}

QString PanelItemTraits::getNodeType(Node *node)
{
    return node->itemType();
}

bool PanelItemTraits::loadFromJSON(Node *node,
                                   const QJsonObject &obj,
                                   ModeManager *modeMgr)
{
    return node->loadFromJSON(obj, modeMgr);
}

void PanelItemTraits::saveToJSON(Node *node, QJsonObject &obj)
{
    node->saveToJSON(obj);
}

void PanelItemTraits::editItem(Node *node, ViewManager *viewMgr, QWidget *parent)
{
    auto editFactory = viewMgr->modeMgr()->panelFactory();
    editFactory->editItem(parent, node, viewMgr, false);
}

QStringList PanelItemTraits::getRegisteredTypes(ModeManager *modeMgr)
{
    auto editFactory = modeMgr->panelFactory();
    return editFactory->getRegisteredTypes();
}

QString PanelItemTraits::prettyTypeName(ModeManager *modeMgr, const QString &typeName)
{
    auto editFactory = modeMgr->panelFactory();
    return editFactory->prettyName(typeName);
}

#include "../../utils/itemobjectreplacedlg_impl.hpp"
