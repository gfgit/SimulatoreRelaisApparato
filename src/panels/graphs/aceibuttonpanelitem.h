/**
 * src/panels/graphs/aceibuttonpanelitem.h
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

#ifndef ACEI_BUTTON_PANELITEM_H
#define ACEI_BUTTON_PANELITEM_H

#include "../snappablepanelitem.h"

class AbstractSimulationObject;
class ButtonInterface;

class LightBulbObject;

class ACEIButtonPanelItem : public SnappablePanelItem
{
    Q_OBJECT
public:
    static constexpr double ItemWidth = 100;
    static constexpr double ItemHeight = 130;

    explicit ACEIButtonPanelItem();
    ~ACEIButtonPanelItem();

    static constexpr QLatin1String ItemType = QLatin1String("acei_button");
    QString itemType() const override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    AbstractSimulationObject *button() const;
    void setButton(AbstractSimulationObject *newButton);

    LightBulbObject *centralLight() const;
    void setCentralLight(LightBulbObject *newCentralLight);

    QColor centralLightColor() const;
    void setCentralLightColor(const QColor &newCentralLightColor);

signals:
    void buttonChanged(AbstractSimulationObject *newButton);
    void lightsChanged();

private slots:
    void onButtonDestroyed();
    void onLightDestroyed();

    void onInterfacePropertyChanged(const QString &ifaceName,
                                    const QString &propName,
                                    const QVariant &value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

private:
    static constexpr double baseCircleRadius = 34;
    static constexpr double buttonCircleRadius = 20;

    static constexpr double lightCircleRadius = 14;
    static constexpr double lightOffset = 20;

private:
    AbstractSimulationObject *mButton = nullptr;
    ButtonInterface *mButtonIface = nullptr;
    LightBulbObject *mCentralLight = nullptr;

    QColor mCentralLightColor = Qt::yellow;
};

#endif // ACEI_BUTTON_PANELITEM_H
