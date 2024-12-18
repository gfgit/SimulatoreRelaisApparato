/**
 * src/panels/graphs/aceileverpanelitem.h
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

#ifndef ACEI_LEVER_PANELITEM_H
#define ACEI_LEVER_PANELITEM_H

#include "../abstractpanelitem.h"

class AbstractSimulationObject;
class LeverInterface;

class LightBulbObject;

class ACEILeverPanelItem : public AbstractPanelItem
{
    Q_OBJECT
public:
    static constexpr double ItemWidth = 100;
    static constexpr double ItemHeight = 130;

    explicit ACEILeverPanelItem();
    ~ACEILeverPanelItem();

    static constexpr QLatin1String ItemType = QLatin1String("acei_lever");
    QString itemType() const override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    AbstractSimulationObject *lever() const;
    void setLever(AbstractSimulationObject *newLever);

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    LightBulbObject *leftLight() const;
    void setLeftLight(LightBulbObject *newLeftLight);

    LightBulbObject *rightLight() const;
    void setRightLight(LightBulbObject *newRightLight);

    QColor leftLightColor() const;
    void setLeftLightColor(const QColor &newLeftLightColor);

    QColor rightLightColor() const;
    void setRightLightColor(const QColor &newRightLightColor);

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
    static constexpr double baseCircleRadius = 34;
    static constexpr double leverCircleRadius = 20;
    static constexpr double leverTipLength = 34;
    static constexpr double leverBottomLength = 26;

    static constexpr double lightCircleRadius = 14;
    static constexpr double lightOffset = 20;

private:
    AbstractSimulationObject *mLever = nullptr;
    LeverInterface *mLeverIface = nullptr;

    LightBulbObject *mLeftLight = nullptr;
    LightBulbObject *mRightLight = nullptr;

    QColor mLeftLightColor = Qt::yellow;
    QColor mRightLightColor = Qt::blue;

    QPointF mLastMousePos;
};

#endif // ACEI_LEVER_PANELITEM_H
