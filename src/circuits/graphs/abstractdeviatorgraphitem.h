/**
 * src/circuits/graphs/abstractdeviatorgraphitem.h
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

#ifndef ABSTRACTDEVIATORGRAPHITEM_H
#define ABSTRACTDEVIATORGRAPHITEM_H

#include "abstractnodegraphitem.h"

class AbstractDeviatorNode;

class AbstractDeviatorGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    static constexpr double morsettiOffset = 22.0;
    static constexpr double arcRadius = 15.0;

public:
    explicit AbstractDeviatorGraphItem(AbstractDeviatorNode *node_);

    void getConnectors(std::vector<Connector>& connectors) const final;

    AbstractDeviatorNode *deviatorNode() const;

protected:
    // Contacts must be already swapped
    void drawDeviator(QPainter *painter, bool contactUpOn, bool contactDownOn);

    virtual void drawCustomArc(QPainter *painter,
                               const QLineF& contact1Line,
                               const QLineF& contact2Line,
                               const QPointF& center);
};

#endif // ABSTRACTDEVIATORGRAPHITEM_H
