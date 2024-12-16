/**
 * src/panels/graphs/imagepanelitem.h
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

#ifndef IMAGEPANELITEM_H
#define IMAGEPANELITEM_H

#include "../abstractpanelitem.h"

class QPixmap;

class ImagePanelItem : public AbstractPanelItem
{
    Q_OBJECT
public:
    explicit ImagePanelItem();
    ~ImagePanelItem();

    static constexpr QLatin1String ItemType = QLatin1String("image_item");
    QString itemType() const override;

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    QPainterPath opaqueArea() const override;

    QString imageFileName() const;
    void setImageFileName(const QString &newImageFileName);

    double imageScale() const;
    void setImageScale(double newScale);

signals:
    void imageScaleChanged(double newScale);

private:
    QString mImageFileName;

    double mImageScale = 1.0;

    QPixmap *mPixmap = nullptr;
};

#endif // IMAGEPANELITEM_H
