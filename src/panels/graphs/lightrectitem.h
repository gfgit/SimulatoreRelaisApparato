/**
 * src/panels/graphs/lightrectitem.h
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

#ifndef LIGHTRECTITEM_H
#define LIGHTRECTITEM_H

#include "../abstractpanelitem.h"

#include <QColor>
#include <QVector>

class AbstractSimulationObject;
class LightBulbObject;

class LightRectItem : public AbstractPanelItem
{
    Q_OBJECT
public:
    struct LightEntry
    {
        LightBulbObject *light = nullptr;
        QColor color = Qt::red;
    };

    explicit LightRectItem();
    ~LightRectItem();

    static constexpr QLatin1String ItemType = QLatin1String("light_rect");
    QString itemType() const override;

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    QRectF rect() const;
    void setRect(const QRectF &newRect);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    QPainterPath opaqueArea() const override;

    bool active() const;

    QVector<LightEntry> lights() const;
    void setLights(const QVector<LightEntry> &newLights);

signals:
    void rectChanged();
    void lightsChanged();

private slots:
    void onLightStateChanged(AbstractSimulationObject *obj);
    void onLightDestroyed(QObject *obj);

private:
    QVector<LightEntry> mLights;

    QRectF mRect = QRectF(0, 0, 20, 20);

    int mActive = 0;
};

inline bool operator==(const LightRectItem::LightEntry& lhs, const LightRectItem::LightEntry& rhs)
{
    return lhs.light == rhs.light && lhs.color == rhs.color;
}

#endif // LIGHTRECTITEM_H
