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
class ButtonInterface;

class LightBulbObject;

class ACESasibLeverPanelItem : public SnappablePanelItem
{
    Q_OBJECT
public:
    static constexpr double ItemWidth = 150;
    static constexpr double ItemHeight = 350;

    enum LightPosition
    {
        Left = 0,
        Right,
        NLights
    };

    enum class MouseState
    {
        None = 0,
        Lever,
        LeftButton,
        RightButton
    };

    static constexpr QLatin1String lightFmt = QLatin1String("light_%1");
    static constexpr QLatin1String lightColorFmt = QLatin1String("light_%1_color");
    static constexpr QLatin1String lightKeyNames[NLights] = {
        QLatin1String("left"),
        QLatin1String("right")
    };

    static constexpr Qt::GlobalColor lightDefaultColors[NLights] = {
        Qt::yellow,
        Qt::blue
    };

    explicit ACESasibLeverPanelItem();
    ~ACESasibLeverPanelItem();

    static constexpr QLatin1String ItemType = QLatin1String("ace_sasib_lever");
    QString itemType() const override;

    QString tooltipString() const override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    void getObjectProperties(QVector<ObjectProperty> &result) const override;

    // Lever
    AbstractSimulationObject *lever() const;
    void setLever(AbstractSimulationObject *newLever);

    // Buttons
    inline AbstractSimulationObject *getButton(LightPosition pos) const
    {
        return mButtons[pos].button;
    }

    void setButton(LightPosition pos, AbstractSimulationObject *newButton);

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

    QColor leverNameColor() const;
    void setLeverNameColor(const QColor &newLeverNameColor);

signals:
    void leverChanged(AbstractSimulationObject *newLever);
    void lightsChanged();

private slots:
    void onLeverDestroyed();
    void onLightDestroyed(QObject *obj);

    void onLeverSettingsChanged();

    void onInterfacePropertyChanged(const QString& ifaceName,
                                    const QString& propName,
                                    const QVariant& value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

private:
    // Lever
    AbstractSimulationObject *mLever = nullptr;
    LeverInterface *mLeverIface = nullptr;

    QColor mLeverNameColor = Qt::black;

    // Buttons
    struct Button
    {
        AbstractSimulationObject *button = nullptr;
        ButtonInterface *buttonIface = nullptr;
    };

    Button mButtons[NLights];

    // Lights
    LightBulbObject *mLights[NLights] = {nullptr, nullptr};
    QColor mLightColors[NLights] = {Qt::yellow, Qt::blue};

    MouseState mMouseState = MouseState::None;
    QPointF mLastMousePos;

    static constexpr QSizeF holeSize = {50, 120};
    static constexpr double holeCenterOffsetY = -30;
    static constexpr double lightCircleRadius = 25;
    static constexpr double buttonCircleRadius = 20;
    static constexpr double buttonBaseCircleRadius = 25;

    static constexpr QPointF LeverHoleCenter = QPointF(ItemWidth / 2,
                                                       250 / 2 + holeCenterOffsetY);

    static constexpr QPointF LeftButCenter = QPointF(buttonBaseCircleRadius * 1.5,
                                                     ItemHeight - buttonBaseCircleRadius * 3.7);
    static constexpr QPointF RightButCenter = QPointF(ItemWidth - buttonBaseCircleRadius * 1.5,
                                                      ItemHeight - buttonBaseCircleRadius * 3.7);

    static constexpr QPointF LeftLightCenter = QPointF(lightCircleRadius * 1.5,
                                                       ItemHeight - lightCircleRadius * 1.5);
    static constexpr QPointF RightLightCenter = QPointF(ItemWidth - lightCircleRadius * 1.5,
                                                        ItemHeight - lightCircleRadius * 1.5);
};

#endif // ACE_SASIB_LEVER_PANELITEM_H
