/**
 * src/circuits/graphs/simpleactivationgraphitem.h
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

#ifndef SIMPLEACTIVATIONGRAPHITEM_H
#define SIMPLEACTIVATIONGRAPHITEM_H

#include "abstractnodegraphitem.h"

class SimpleActivationNode;
class AbstractSimpleActivableObject;

class SimpleActivationGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    explicit SimpleActivationGraphItem(SimpleActivationNode *node_);

    void getConnectors(std::vector<Connector>& connectors) const final;

    QString displayString() const override;

    QString tooltipString() const override;

    SimpleActivationNode *activationNode() const;

private slots:
    void setObject(AbstractSimpleActivableObject *obj);

protected:
    static constexpr double circleRadius = 50 - 10/2; // Half rect - half pen width

    AbstractSimpleActivableObject *mActivableObj = nullptr;
};

#endif // SIMPLEACTIVATIONGRAPHITEM_H
