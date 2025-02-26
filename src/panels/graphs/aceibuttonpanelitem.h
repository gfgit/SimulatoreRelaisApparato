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

    enum LightPosition
    {
        Left = 0,
        Central,
        Right,
        NLights
    };

    static constexpr QLatin1String lightFmt = QLatin1String("light_%1");
    static constexpr QLatin1String lightColorFmt = QLatin1String("light_%1_color");
    static constexpr QLatin1String lightKeyNames[NLights] = {
        QLatin1String("left"),
        QLatin1String("central"),
        QLatin1String("right")
    };

    static constexpr Qt::GlobalColor lightDefaultColors[NLights] = {
        Qt::yellow,
        Qt::yellow,
        Qt::blue
    };

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

    // Lights
    inline LightBulbObject *getLight(LightPosition pos) const
    {
        return mLights[pos];
    }

    void setLight(LightPosition pos, LightBulbObject *newLight);

    inline QColor getLightColor(LightPosition pos) const
    {
        return mLightColors[pos];
    }

    void setLightColor(LightPosition pos, const QColor &newLightColor);

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

    static constexpr double lightCircleRadius = 13;
    static constexpr double lightOffsetX = 20;
    static constexpr double lightOffsetY = 24;
    static constexpr double lightOffsetCentralY = 17;

private:
    AbstractSimulationObject *mButton = nullptr;
    ButtonInterface *mButtonIface = nullptr;

    // Lights
    LightBulbObject *mLights[NLights] = {nullptr, nullptr, nullptr};
    QColor mLightColors[NLights] = {Qt::yellow, Qt::yellow, Qt::blue};
};

#endif // ACEI_BUTTON_PANELITEM_H
