/**
 * src/circuits/graphs/special/aceilevergraphitem.h
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

#ifndef ACEILEVERGRAPHITEM_H
#define ACEILEVERGRAPHITEM_H

#include "../abstractnodegraphitem.h"

#include "../../nodes/onoffswitchnode.h"

class AbstractSimulationObject;
class LeverInterface;

class LightBulbObject;

// TODO: this is a fake node
class FakeLeverNode : public OnOffSwitchNode
{
public:
    explicit FakeLeverNode(ModeManager *mgr, QObject *parent = nullptr)
        : OnOffSwitchNode(mgr, parent)
    {

    }

    static constexpr QLatin1String NodeType = QLatin1String("acei_lever");
    QString nodeType() const override;
};

class ACEILeverGraphItem : public AbstractNodeGraphItem
{
    Q_OBJECT
public:
    typedef FakeLeverNode Node;
    static constexpr QLatin1String CustomNodeType = QLatin1String("acei_lever");

    explicit ACEILeverGraphItem(OnOffSwitchNode *node_);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    AbstractSimulationObject *lever() const;
    void setLever(AbstractSimulationObject *newLever);

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    LightBulbObject *leftLight() const;
    void setLeftLight(LightBulbObject *newLeftLight);

    LightBulbObject *rightLight() const;
    void setRightLight(LightBulbObject *newRightLight);

signals:
    void leverChanged(AbstractSimulationObject *newLever);
    void lightsChanged();

private slots:
    void onLeverDestroyed();
    void onLightDestroyed();

    void onInterfacePropertyChanged(const QString &ifaceName,
                                    const QString &propName,
                                    const QVariant &value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

private:
    void updateLeverTooltip();

private:
    AbstractSimulationObject *mLever = nullptr;
    LeverInterface *mLeverIface = nullptr;

    LightBulbObject *mLeftLight = nullptr;
    LightBulbObject *mRightLight = nullptr;

    QPointF mLastMousePos;
};

#endif // ACEILEVERGRAPHITEM_H
