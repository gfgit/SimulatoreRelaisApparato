/**
 * src/circuits/nodes/bifilarizatornode.h
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

#ifndef BIFILARIZATORNODE_H
#define BIFILARIZATORNODE_H

#include "abstractcircuitnode.h"

class BifilarizatorNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit BifilarizatorNode(ModeManager *mgr, QObject *parent = nullptr);

    ConnectionsRes getActiveConnections(CableItem source, bool invertDir) override;

    static constexpr QLatin1String NodeType = QLatin1String("bifilarizator_node");
    QString nodeType() const override;

public:
    enum Contacts
    {
        FirstPoleContact = 2,
        CentralContact = 1,
        SecondPoleContact = 0
    };
};

#endif // BIFILARIZATORNODE_H
