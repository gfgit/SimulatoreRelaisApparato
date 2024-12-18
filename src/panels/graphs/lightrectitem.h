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

class LightBulbObject;

class LightRectItem : public AbstractPanelItem
{
    Q_OBJECT
public:
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
    void setActive(bool newActive);

    QColor color() const;
    void setColor(const QColor &newColor);

    LightBulbObject *lightObject() const;
    void setLightObject(LightBulbObject *newLightObject);

signals:
    void colorChanged();
    void rectChanged();
    void lightChanged();

private slots:
    void onLightStateChanged();

private:
    QRectF mRect = QRectF(0, 0, 20, 20);
    QColor mColor = Qt::red;

    LightBulbObject *mLightObject = nullptr;

    bool mActive = false;
};

#endif // LIGHTRECTITEM_H
