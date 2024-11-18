/**
 * src/circuits/graphs/special/acesasiblevergraphitem.h
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

#ifndef ACESASIBLEVERGRAPHITEM_H
#define ACESASIBLEVERGRAPHITEM_H

#include "../abstractnodegraphitem.h"

#include "../../nodes/onoffswitchnode.h"

class GenericLeverObject;

// TODO: this is a fake node
class FakeLeverNode2 : public OnOffSwitchNode
{
public:
    explicit FakeLeverNode2(ModeManager *mgr, QObject *parent = nullptr)
        : OnOffSwitchNode(mgr, parent)
    {

    }

    static constexpr QLatin1String NodeType = QLatin1String("ace_sasib_lever");
    QString nodeType() const override;
};

class ACESasibLeverGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef FakeLeverNode2 Node;
    static constexpr QLatin1String CustomNodeType = QLatin1String("ace_sasib_lever");

    explicit ACESasibLeverGraphItem(OnOffSwitchNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    GenericLeverObject *lever() const;
    void setLever(GenericLeverObject *newLever);

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

signals:
    void leverChanged(GenericLeverObject *newLever);

private slots:
    void onLeverDestroyed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

private:
    GenericLeverObject *mLever = nullptr;

    QPointF mLastMousePos;
};

#endif // ACESASIBLEVERGRAPHITEM_H
