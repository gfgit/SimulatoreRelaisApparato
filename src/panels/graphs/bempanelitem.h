/**
 * src/panels/graphs/bempanelitem.h
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

#ifndef BEM_PANELITEM_H
#define BEM_PANELITEM_H

#include "../snappablepanelitem.h"

class AbstractSimulationObject;
class BEMLeverObject;
class LightBulbObject;

class BEMHandleInterface;
class LeverInterface;
class ButtonInterface;

class BEMPanelItem : public SnappablePanelItem
{
    Q_OBJECT
public:
    static constexpr double ItemWidth = 400;
    static constexpr double ItemHeight = 680;

    enum class MouseState
    {
        None = 0,
        TxButton,
        ArtificialLibButton,
        LightButton,
        RequestLever,
        ConsensusLever
    };

    explicit BEMPanelItem();
    ~BEMPanelItem();

    static constexpr QLatin1String ItemType = QLatin1String("bem");
    QString itemType() const override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget = nullptr) override;

    bool loadFromJSON(const QJsonObject& obj, ModeManager *mgr) override;
    void saveToJSON(QJsonObject& obj) const override;

    void setConsensusLever(BEMLeverObject *consLever);
    inline BEMLeverObject *getConsensusLever() const { return mConsLeverObj; }

    void setTxButton(AbstractSimulationObject *newButton);
    AbstractSimulationObject *getTxButton();

private slots:
    void onTxButDestroyed();
    void onConsLeverDestroyed();
    void onConsLeverInterfaceChanged(const QString &ifaceName, const QString &propName, const QVariant &value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;

private:
    void setRequestLever(BEMLeverObject *reqLever);

    void setArtLibButton(AbstractSimulationObject *newButton);

signals:
    void settingsChanged();

private:
    static constexpr double innerSmallRadius = 11;
    static constexpr QPointF LightButtonCenter = QPointF(ItemWidth - 109, 336);

    static constexpr double artificialLiberationRadius = 25;
    static constexpr QPointF ArtLibCenter = QPointF(ItemWidth / 2, 350 + artificialLiberationRadius);

    static constexpr double leverRectBaseRadius = 25;
    static constexpr double leverLength = 75;
    static constexpr QPointF ReqLeverCenter = QPointF(77 + leverRectBaseRadius,
                                                      457 + leverRectBaseRadius);
    static constexpr QPointF ConsLeverCenter = QPointF(ItemWidth - 77 - leverRectBaseRadius,
                                                       457 + leverRectBaseRadius);

private:
    BEMLeverObject *mReqLeverObj = nullptr;
    BEMLeverObject *mConsLeverObj = nullptr;

    LeverInterface *mReqLever = nullptr;
    LeverInterface *mConsLever = nullptr;

    LightBulbObject *mLight = nullptr;

    ButtonInterface *mTxButton = nullptr;
    ButtonInterface *mArtificialLibBut = nullptr;
    ButtonInterface *mLightButton = nullptr;

    MouseState mMouseState = MouseState::None;
    QPointF mLastMousePos;
};

#endif // BEM_PANELITEM_H
