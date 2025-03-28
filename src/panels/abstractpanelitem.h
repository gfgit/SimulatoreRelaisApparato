/**
 * src/panels/abstractpanelitem.h
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

#ifndef ABSTRACT_PANEL_ITEM_H
#define ABSTRACT_PANEL_ITEM_H

#include <QGraphicsObject>

#include "../utils/objectproperty.h"

class PanelScene;
class ModeManager;

class QJsonObject;

class AbstractPanelItem : public QGraphicsObject
{
    Q_OBJECT
public:
    static constexpr QRgb SelectedBackground = qRgb(180, 255, 255);

    AbstractPanelItem();

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    QPainterPath opaqueArea() const override;

    PanelScene *panelScene() const;

    virtual bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr);
    virtual void saveToJSON(QJsonObject& obj) const;

    virtual void getObjectProperties(QVector<ObjectProperty> &result) const;

    virtual QString itemType() const = 0;

    virtual QString tooltipString() const;

protected slots:
    void triggerUpdate();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif // ABSTRACT_PANEL_ITEM_H
