/**
 * src/panels/graphs/imagepanelitem.cpp
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

#include "imagepanelitem.h"

#include <QPainter>
#include <QPixmap>

#include <QFileInfo>
#include <QDir>
#include <QJsonObject>

#include "../panelscene.h"
#include "../../views/modemanager.h"

ImagePanelItem::ImagePanelItem()
    : AbstractPanelItem()
{

}

ImagePanelItem::~ImagePanelItem()
{
    if(mPixmap)
    {
        delete mPixmap;
        mPixmap = nullptr;
    }
}

QString ImagePanelItem::itemType() const
{
    return ItemType;
}

bool ImagePanelItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!AbstractPanelItem::loadFromJSON(obj, mgr))
        return false;

    QString imagePath = obj.value("image_file").toString();
    if(!imagePath.isEmpty())
    {
        QFileInfo imageFileInfo(imagePath);
        if(imageFileInfo.isRelative())
        {
            const QString jsonPath = mgr->filePath();
            QFileInfo info(jsonPath);
            imagePath = info.absoluteDir().absoluteFilePath(imagePath);
        }
    }

    setImageFileName(imagePath);
    setImageScale(obj.value("scale").toDouble(1.0));

    return true;
}

void ImagePanelItem::saveToJSON(QJsonObject &obj) const
{
    AbstractPanelItem::saveToJSON(obj);

    QString imagePath = imageFileName();

    const QString jsonPath = panelScene() ?
                panelScene()->modeMgr()->filePath() :
                QString();

    if(!jsonPath.isEmpty())
    {
        QFileInfo info(jsonPath);
        if(info.exists())
        {
            imagePath = info.absoluteDir().relativeFilePath(imagePath);
        }
    }

    obj["image_file"] = imagePath;
    obj["scale"] = mImageScale;
}

QRectF ImagePanelItem::boundingRect() const
{
    if(mPixmap && !mPixmap->isNull())
    {
        return QRectF(QPointF(),
                      QSizeF(mPixmap->size()) * mImageScale / mPixmap->devicePixelRatio());
    }

    // Null don't be invisible
    return QRectF(QPointF(), QSizeF(20, 20));
}

void ImagePanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(mPixmap)
    {
        const QRectF target(QPointF(),
                            QSizeF(mPixmap->size()) * mImageScale / mPixmap->devicePixelRatio());

        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter->drawPixmap(target, *mPixmap, QRectF());
    }
    else
    {
        painter->fillRect(boundingRect(), Qt::red);
    }
}

QPainterPath ImagePanelItem::opaqueArea() const
{
    return shape();
}

QString ImagePanelItem::imageFileName() const
{
    return mImageFileName;
}

void ImagePanelItem::setImageFileName(const QString &newImageFileName)
{
    if(mImageFileName == newImageFileName)
        return;

    QFileInfo info(newImageFileName);
    mImageFileName = info.canonicalFilePath();

    // Load image
    prepareGeometryChange();

    if(info.suffix().toLower() == "svg")
    {
        if(mPixmap)
        {
            delete mPixmap;
            mPixmap = nullptr;
        }
    }
    else
    {
        if(!mPixmap)
            mPixmap = new QPixmap;

        mPixmap->load(mImageFileName);
    }

    update();

    if(panelScene())
        panelScene()->modeMgr()->setFileEdited();
}

double ImagePanelItem::imageScale() const
{
    return mImageScale;
}

void ImagePanelItem::setImageScale(double newScale)
{
    newScale = qBound(0.2, newScale, 4.0);

    if(mImageScale == newScale)
        return;

    prepareGeometryChange();
    mImageScale = newScale;
    update();

    if(panelScene())
        panelScene()->modeMgr()->setFileEdited();

    emit imageScaleChanged(newScale);
}
