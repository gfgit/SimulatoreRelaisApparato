/**
 * src/circuits/view/circuitnodeobjectreplacedlg.h
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

#ifndef CIRCUIT_NODE_OBJECT_REPLACE_DLG_H
#define CIRCUIT_NODE_OBJECT_REPLACE_DLG_H

#include "../../utils/itemobjectreplacedlg.h"

class AbstractNodeGraphItem;

class ModeManager;
class ViewManager;

class ObjectProperty;

struct CircuitNodeTraits
{
    typedef AbstractNodeGraphItem Node;

    static void getObjectProperties(Node *node,
                                    QVector<ObjectProperty> &result);
    static QString getNodeType(Node *node);

    static bool loadFromJSON(Node *node, const QJsonObject& obj);
    static void saveToJSON(Node *node, QJsonObject& obj);

    static void editItem(Node *node,
                         ViewManager *viewMgr,
                         QWidget *parent);

    static QStringList getRegisteredTypes(ModeManager *modeMgr);
    static QString prettyTypeName(ModeManager *modeMgr, const QString& typeName);
};

class CircuitNodeObjectReplaceDlg : public ItemObjectReplaceDlg<CircuitNodeTraits>
{
public:
    CircuitNodeObjectReplaceDlg(ViewManager *viewMgr,
                                const QVector<AbstractNodeGraphItem *>& items,
                                QWidget *parent = nullptr);
};

#endif // CIRCUIT_NODE_OBJECT_REPLACE_DLG_H
