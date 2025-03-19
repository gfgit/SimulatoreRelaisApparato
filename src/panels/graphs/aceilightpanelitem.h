/**
 * src/panels/graphs/aceilightpanelitem.h
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

#ifndef ACEI_LIGHT_PANELITEM_H
#define ACEI_LIGHT_PANELITEM_H

#include "../snappablepanelitem.h"

class LightBulbObject;

class ACEILightPanelItem : public SnappablePanelItem
{
    Q_OBJECT
public:
    static constexpr double ItemWidth = 100;
    static constexpr double ItemHeight = 130;

    explicit ACEILightPanelItem();
    ~ACEILightPanelItem();

    static constexpr QLatin1String ItemType = QLatin1String("acei_light");
    QString itemType() const override;

    QString tooltipString() const override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    inline LightBulbObject *getLight() const
    {
        return mLight;
    }

    void setLight(LightBulbObject *newLight);

    inline QColor getLightColor() const
    {
        return mLightColor;
    }

    void setLightColor(const QColor &newLightColor);

signals:
    void lightsChanged();

private slots:
    void onLightDestroyed();

private:
    // Base is slightly smaller than lever/button base
    static constexpr double baseCircleRadius = 28;
    static constexpr double lightCircleRadius = 13;

private:
    LightBulbObject *mLight = nullptr;
    QColor mLightColor = Qt::yellow;
};

#endif // ACEI_LIGHT_PANELITEM_H
