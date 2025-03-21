/**
 * src/panels/graphs/acesasibleverpanelitem.h
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

#ifndef ACE_SASIB_LEVER_PANELITEM_H
#define ACE_SASIB_LEVER_PANELITEM_H

#include "../snappablepanelitem.h"

class AbstractSimulationObject;
class LeverInterface;

class ACESasibLeverPanelItem : public SnappablePanelItem
{
    Q_OBJECT
public:
    static constexpr double ItemWidth = 150;
    static constexpr double ItemHeight = 250;

    explicit ACESasibLeverPanelItem();
    ~ACESasibLeverPanelItem();

    static constexpr QLatin1String ItemType = QLatin1String("ace_sasib_lever");
    QString itemType() const override;

    QString tooltipString() const override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    AbstractSimulationObject *lever() const;
    void setLever(AbstractSimulationObject *newLever);

signals:
    void leverChanged(AbstractSimulationObject *newLever);

private slots:
    void onLeverDestroyed();

    void onInterfacePropertyChanged(const QString& ifaceName,
                                    const QString& propName,
                                    const QVariant& value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

private:
    AbstractSimulationObject *mLever = nullptr;
    LeverInterface *mLeverIface = nullptr;

    QPointF mLastMousePos;

    static constexpr QSizeF holeSize = {50, 120};
    static constexpr double holeCenterOffsetY = -30;

};

#endif // ACE_SASIB_LEVER_PANELITEM_H
