/**
 * src/panels/graphs/bempanelitem.cpp
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

#include "bempanelitem.h"
#include "../panelscene.h"

#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../objects/interfaces/bemhandleinterface.h"
#include "../../objects/interfaces/buttoninterface.h"
#include "../../objects/interfaces/leverinterface.h"

#include "../../objects/relais/model/abstractrelais.h"
#include "../../objects/simple_activable/lightbulbobject.h"
#include "../../objects/lever/bem/bemleverobject.h"

#include "../../views/modemanager.h"

#include <QPainter>

#include <QJsonObject>

#include <QGraphicsSceneMouseEvent>

BEMPanelItem::BEMPanelItem()
    : SnappablePanelItem()
{

}

BEMPanelItem::~BEMPanelItem()
{
    setConsensusLever(nullptr);
}

QString BEMPanelItem::itemType() const
{
    return ItemType;
}

QRectF BEMPanelItem::boundingRect() const
{
    return QRectF(0, 0, ItemWidth, ItemHeight);
}

void BEMPanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QRectF br = boundingRect();

    // Background
    painter->fillRect(br, isSelected() ? SelectedBackground : qRgb(0x7F, 0x7F, 0x7F));

    // // Zero is vertical up, so cos/sin are swapped
    // // Also returned angle must be inverted to be clockwise
    // const double angleRadiants = -qDegreesToRadians(mLeverIface ? mLeverIface->angle() : 0);
    // const QPointF delta(qSin(angleRadiants),
    //                     qCos(angleRadiants));

    QRectF baseRect = br;
    baseRect.setTop(br.bottom() - 140);

    QRectF caseRect = br;
    caseRect.setBottom(baseRect.top() + 22);

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::red);
    painter->drawRoundedRect(caseRect, 22, 22);

    painter->setBrush(Qt::black);
    painter->drawRect(baseRect);

    const double circleRadius = 65;
    const double innerCircleRadius = 50;
    const double statusCircleRadius = 30;
    QRectF circle(0, 0, circleRadius * 2, circleRadius * 2);

    QRectF innerCircle(0, 0, innerCircleRadius * 2, innerCircleRadius * 2);
    QRectF statusCircle(0, 0, statusCircleRadius * 2, statusCircleRadius * 2);

    QPen barPen;
    barPen.setColor(Qt::black);
    barPen.setWidthF(10);
    barPen.setCapStyle(Qt::FlatCap);

    const double sqrtFactor = 0.7; // 1 / sqrt(2) = cos(45)
    const QPointF statusLineDiff(statusCircleRadius * sqrtFactor, - statusCircleRadius * sqrtFactor);

    // A1
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    circle.moveCenter(QPointF(40 + circleRadius, 18 + circleRadius));
    innerCircle.moveCenter(circle.center());
    statusCircle.moveCenter(circle.center());
    painter->drawEllipse(circle);
    painter->drawEllipse(innerCircle);

    const bool hasRecvConsensus = mR1Relay && mR1Relay->state() == AbstractRelais::State::Down;
    if(hasRecvConsensus)
        painter->setBrush(Qt::white);
    else
        painter->setBrush(Qt::red);
    painter->drawEllipse(statusCircle);

    const bool canUseConsensus = mOccupancyRelay && mOccupancyRelay->state() == AbstractRelais::State::Up;
    if(!canUseConsensus)
    {
        painter->setPen(barPen);
        painter->drawLine(statusCircle.center() + statusLineDiff,
                          statusCircle.center() - statusLineDiff);
    }

    // A2
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    circle.moveCenter(QPointF(br.right() - (40 + circleRadius), 18 + circleRadius));
    innerCircle.moveCenter(circle.center());
    statusCircle.moveCenter(circle.center());
    painter->drawEllipse(circle);
    painter->drawEllipse(innerCircle);

    const bool hasSentConsensus = mC1Relay && mC1Relay->state() == AbstractRelais::State::Down;
    if(hasSentConsensus)
        painter->setBrush(Qt::darkGreen);
    else
        painter->setBrush(Qt::red);
    painter->drawEllipse(statusCircle);

    const bool canSendConsensus = mKConditionsRelay && mKConditionsRelay->state() == AbstractRelais::State::Up;
    if(!canSendConsensus)
    {
        const QPointF invertedLineDiff(statusLineDiff.x(),
                                       -statusLineDiff.y());

        painter->setPen(barPen);
        painter->drawLine(statusCircle.center() + invertedLineDiff,
                          statusCircle.center() - invertedLineDiff);
    }

    // A3
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    circle.moveCenter(QPointF(br.center().x(), 110 + circleRadius));
    innerCircle.moveCenter(circle.center());
    statusCircle.moveCenter(circle.center());
    painter->drawEllipse(circle);
    painter->drawEllipse(innerCircle);

    // A3 Status

    // If liberation relay is not fully Up, lever stays locked
    bool blocked = mLiberationRelay && mLiberationRelay->state() != AbstractRelais::State::Up;

    if(blocked)
        painter->setBrush(Qt::red);
    else
        painter->setBrush(Qt::darkGreen);

    painter->drawEllipse(statusCircle);

    // Sigillo
    QRectF sigilloCircle(112, 230,
                         smallRadius * 2, smallRadius * 2);

    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(sigilloCircle);

    sigilloCircle.moveRight(br.right() - sigilloCircle.left());
    painter->drawEllipse(sigilloCircle);

    // Label
    QRectF labelRect(br.center().x() - 65, 265, 130, 66);
    painter->setPen(Qt::white);
    painter->setBrush(Qt::lightGray);

    painter->drawRect(labelRect);

    // Light
    QRectF lightCircle(0, 0,
                       smallRadius * 2, smallRadius * 2);

    lightCircle.moveCenter(LightCenter);
    painter->setPen(Qt::white);
    QRectF innerLightCircle(0, 0, innerSmallRadius * 2, innerSmallRadius * 2);
    innerLightCircle.moveCenter(lightCircle.center());

    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(lightCircle);

    if(mLight && mLight->state() == LightBulbObject::State::On)
        painter->setBrush(Qt::yellow);
    else
        painter->setBrush(Qt::lightGray);
    painter->drawEllipse(innerLightCircle);


    // Light button
    lightCircle.moveCenter(LightButtonCenter);
    if(mLightButton && mLightButton->state() == ButtonInterface::State::Pressed)
        innerLightCircle.setSize(innerLightCircle.size() * 0.8); // Draw smaller when pressed
    innerLightCircle.moveCenter(lightCircle.center());

    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(lightCircle);

    painter->setBrush(Qt::black);
    painter->drawEllipse(innerLightCircle);

    // Artificial Liberation Button
    QRectF artLibCircle(0, 0,
                        artificialLiberationRadius * 2, artificialLiberationRadius * 2);
    artLibCircle.moveCenter(ArtLibCenter);

    painter->setPen(Qt::darkMagenta);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(artLibCircle);

    double artLibButRadius = artificialLiberationRadius * 0.7;
    if(mArtificialLibBut && mArtificialLibBut->state() == ButtonInterface::State::Pressed)
        artLibButRadius = artificialLiberationRadius * 0.5;

    QRectF artLibInnerCircle(0, 0, artLibButRadius * 2, artLibButRadius * 2);
    artLibInnerCircle.moveCenter(ArtLibCenter);

    painter->setPen(Qt::black);
    painter->setBrush(Qt::lightGray);
    painter->drawEllipse(artLibInnerCircle);

    // Lever Handles
    QRectF leverBase(0, 0, leverRectBaseRadius * 2, leverRectBaseRadius * 2);
    leverBase.moveCenter(ReqLeverCenter);

    QRect leverRect(-15, 0, 30, leverLength);
    leverRect.moveBottom(-leverRectBaseRadius);

    const int reqLeverAngle = mReqLever ? mReqLever->angle() : 180;
    const int consLeverAngle = mConsLever ? mConsLever->angle() : 180;

    const double txButRadius = mTxButton && mTxButton->state() == ButtonInterface::State::Pressed ?
                10 : 15;
    QRectF txButCircle(0, 0, txButRadius * 2, txButRadius * 2);
    txButCircle.moveCenter(leverBase.center());

    // Request lever
    painter->setPen(Qt::black);
    painter->setBrush(Qt::darkGray);

    painter->drawEllipse(leverBase);

    painter->save();
    painter->translate(leverBase.center());
    painter->rotate(reqLeverAngle);
    painter->drawRect(leverRect);
    painter->restore();

    // Tx Button 1
    painter->setBrush(Qt::black);
    painter->drawEllipse(txButCircle);

    // Consensus lever
    painter->setPen(Qt::black);
    painter->setBrush(Qt::darkGray);

    leverBase.moveRight(br.right() - leverBase.left());
    painter->drawEllipse(leverBase);

    painter->save();
    painter->translate(leverBase.center());
    painter->rotate(consLeverAngle);
    painter->drawRect(leverRect);
    painter->restore();

    // Tx Button 2
    painter->setBrush(Qt::black);
    txButCircle.moveCenter(leverBase.center());
    painter->drawEllipse(txButCircle);
}

bool BEMPanelItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!SnappablePanelItem::loadFromJSON(obj, mgr))
        return false;

    const QString consLeverName = obj.value("cons_lever").toString();
    auto bemModel = mgr->modelForType(BEMLeverObject::Type);
    setConsensusLever(static_cast<BEMLeverObject *>(bemModel->getObjectByName(consLeverName)));

    const QString txButName = obj.value("tx_button").toString();
    const QString txButType = obj.value("tx_button_type").toString();
    auto txButModel = mgr->modelForType(txButType);

    if(txButModel)
        setTxButton(txButModel->getObjectByName(txButName));
    else
        setTxButton(nullptr);

    const QString lightButName = obj.value("light_button").toString();
    const QString lightButType = obj.value("light_button_type").toString();
    auto lightButModel = mgr->modelForType(lightButType);

    if(lightButModel)
        setLightButton(lightButModel->getObjectByName(lightButName));
    else
        setLightButton(nullptr);

    const QString lightName = obj.value("light").toString();
    auto lightModel = mgr->modelForType(LightBulbObject::Type);
    setLight(static_cast<LightBulbObject *>(lightModel->getObjectByName(lightName)));


    // Relays
    auto relayModel = mgr->modelForType(AbstractRelais::Type);

    const QString R1Name = obj.value("relay_R1").toString();
    setR1Relay(static_cast<AbstractRelais *>(relayModel->getObjectByName(R1Name)));

    const QString C1Name = obj.value("relay_C1").toString();
    setC1Relay(static_cast<AbstractRelais *>(relayModel->getObjectByName(C1Name)));

    const QString HName = obj.value("relay_H").toString();
    setOccupancyRelay(static_cast<AbstractRelais *>(relayModel->getObjectByName(HName)));

    const QString KName = obj.value("relay_K").toString();
    setKConditionsRelay(static_cast<AbstractRelais *>(relayModel->getObjectByName(KName)));

    return true;
}

void BEMPanelItem::saveToJSON(QJsonObject &obj) const
{
    SnappablePanelItem::saveToJSON(obj);

    obj["cons_lever"] = mConsLeverObj ? mConsLeverObj->name() : QString();

    obj["tx_button"] = mTxButton ? mTxButton->object()->name() : QString();
    obj["tx_button_type"] = mTxButton ? mTxButton->object()->getType() : QString();

    obj["light_button"] = mLightButton ? mLightButton->object()->name() : QString();
    obj["light_button_type"] = mLightButton ? mLightButton->object()->getType() : QString();

    obj["light"] = mLight ? mLight->name() : QString();

    obj["relay_R1"] = mR1Relay ? mR1Relay->name() : QString();
    obj["relay_C1"] = mC1Relay ? mC1Relay->name() : QString();
    obj["relay_H"] = mOccupancyRelay ? mOccupancyRelay->name() : QString();
    obj["relay_K"] = mKConditionsRelay ? mKConditionsRelay->name() : QString();
}

void BEMPanelItem::setConsensusLever(BEMLeverObject *consLever)
{
    if(mConsLeverObj == consLever)
        return;

    if(mConsLeverObj)
    {
        disconnect(mConsLeverObj, &AbstractSimulationObject::destroyed,
                   this, &BEMPanelItem::onConsLeverDestroyed);
        disconnect(mConsLeverObj, &AbstractSimulationObject::stateChanged,
                   this, &BEMPanelItem::triggerUpdate);
        disconnect(mConsLeverObj, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &BEMPanelItem::onConsLeverInterfaceChanged);
        disconnect(mConsLeverObj, &AbstractSimulationObject::settingsChanged,
                   this, &BEMPanelItem::triggerUpdate);
        mConsLever = nullptr;
    }

    mConsLeverObj = consLever;

    if(mConsLeverObj)
    {
        connect(mConsLeverObj, &AbstractSimulationObject::destroyed,
                this, &BEMPanelItem::onConsLeverDestroyed);
        connect(mConsLeverObj, &AbstractSimulationObject::stateChanged,
                this, &BEMPanelItem::triggerUpdate);
        connect(mConsLeverObj, &AbstractSimulationObject::interfacePropertyChanged,
                this, &BEMPanelItem::onConsLeverInterfaceChanged);
        connect(mConsLeverObj, &AbstractSimulationObject::settingsChanged,
                this, &BEMPanelItem::triggerUpdate);

        mConsLever = mConsLeverObj->getInterface<LeverInterface>();
    }

    BEMHandleInterface *consLeverIface = mConsLeverObj ?
                mConsLeverObj->getInterface<BEMHandleInterface>() :
                nullptr;

    BEMHandleInterface *reqLever = consLeverIface ?
                consLeverIface->getTwinHandle() :
                nullptr;
    setRequestLever(reqLever ? static_cast<BEMLeverObject *>(reqLever->object()) : nullptr);

    ButtonInterface *artLibBut = consLeverIface ?
                consLeverIface->artificialLiberation() : nullptr;
    setArtLibButton(artLibBut ? artLibBut->object() : nullptr);

    AbstractRelais *libRelay = consLeverIface ?
                consLeverIface->liberationRelay() : nullptr;
    setLiberationRelay(libRelay);

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    //updateLeverTooltip();

    emit settingsChanged();

    update();
}

void BEMPanelItem::setRequestLever(BEMLeverObject *reqLever)
{
    if(mReqLeverObj == reqLever)
        return;

    if(mReqLeverObj)
    {
        disconnect(mReqLeverObj, &AbstractSimulationObject::stateChanged,
                   this, &BEMPanelItem::triggerUpdate);
        mReqLever = nullptr;
    }

    mReqLeverObj = reqLever;

    if(mReqLeverObj)
    {
        connect(mReqLeverObj, &AbstractSimulationObject::stateChanged,
                this, &BEMPanelItem::triggerUpdate);

        mReqLever = mReqLeverObj->getInterface<LeverInterface>();
    }

    update();
}

void BEMPanelItem::setTxButton(AbstractSimulationObject *newButton)
{
    ButtonInterface *butIface = newButton ? newButton->getInterface<ButtonInterface>() : nullptr;

    if(butIface == mTxButton)
        return;

    if(mTxButton)
    {
        AbstractSimulationObject *oldBut = mTxButton->object();

        disconnect(oldBut, &AbstractSimulationObject::destroyed,
                   this, &BEMPanelItem::onTxButDestroyed);
        disconnect(oldBut, &AbstractSimulationObject::stateChanged,
                   this, &BEMPanelItem::triggerUpdate);
        disconnect(oldBut, &AbstractSimulationObject::settingsChanged,
                   this, &BEMPanelItem::triggerUpdate);
        mTxButton = nullptr;
    }

    if(newButton && butIface)
    {
        connect(newButton, &AbstractSimulationObject::destroyed,
                this, &BEMPanelItem::onTxButDestroyed);
        connect(newButton, &AbstractSimulationObject::stateChanged,
                this, &BEMPanelItem::triggerUpdate);
        connect(newButton, &AbstractSimulationObject::settingsChanged,
                this, &BEMPanelItem::triggerUpdate);

        mTxButton = butIface;
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit settingsChanged();
}

AbstractSimulationObject *BEMPanelItem::getTxButton()
{
    return mTxButton ? mTxButton->object() : nullptr;
}

void BEMPanelItem::onTxButDestroyed()
{
    setTxButton(nullptr);
}

void BEMPanelItem::setLightButton(AbstractSimulationObject *newButton)
{
    ButtonInterface *butIface = newButton ? newButton->getInterface<ButtonInterface>() : nullptr;

    if(butIface == mLightButton)
        return;

    if(mLightButton)
    {
        AbstractSimulationObject *oldBut = mLightButton->object();

        disconnect(oldBut, &AbstractSimulationObject::destroyed,
                   this, &BEMPanelItem::onLightButDestroyed);
        disconnect(oldBut, &AbstractSimulationObject::stateChanged,
                   this, &BEMPanelItem::triggerUpdate);
        disconnect(oldBut, &AbstractSimulationObject::settingsChanged,
                   this, &BEMPanelItem::triggerUpdate);
        mLightButton = nullptr;
    }

    if(newButton && butIface)
    {
        connect(newButton, &AbstractSimulationObject::destroyed,
                this, &BEMPanelItem::onLightButDestroyed);
        connect(newButton, &AbstractSimulationObject::stateChanged,
                this, &BEMPanelItem::triggerUpdate);
        connect(newButton, &AbstractSimulationObject::settingsChanged,
                this, &BEMPanelItem::triggerUpdate);

        mLightButton = butIface;
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit settingsChanged();
}

AbstractSimulationObject *BEMPanelItem::getLightButton()
{
    return mLightButton ? mLightButton->object() : nullptr;
}

void BEMPanelItem::onLightButDestroyed()
{
    setLightButton(nullptr);
}

void BEMPanelItem::setLight(LightBulbObject *newCentralLight)
{
    if(mLight == newCentralLight)
        return;

    if(mLight)
    {
        disconnect(mLight, &LightBulbObject::stateChanged,
                   this, &BEMPanelItem::triggerUpdate);
        disconnect(mLight, &LightBulbObject::destroyed,
                   this, &BEMPanelItem::onLightDestroyed);
    }

    mLight = newCentralLight;

    if(mLight)
    {
        connect(mLight, &LightBulbObject::stateChanged,
                this, &BEMPanelItem::triggerUpdate);
        connect(mLight, &LightBulbObject::destroyed,
                this, &BEMPanelItem::onLightDestroyed);
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit settingsChanged();
}

LightBulbObject *BEMPanelItem::getLight() const
{
    return mLight;
}

void BEMPanelItem::onLightDestroyed()
{
    setLight(nullptr);
}

void BEMPanelItem::onRelayDestroyed()
{
    if(sender() == mC1Relay)
        setC1Relay(nullptr);
    if(sender() == mR1Relay)
        setR1Relay(nullptr);
    if(sender() == mOccupancyRelay)
        setOccupancyRelay(nullptr);
    if(sender() == mKConditionsRelay)
        setKConditionsRelay(nullptr);
    if(sender() == mLiberationRelay)
        setLiberationRelay(nullptr);
}

void BEMPanelItem::setLiberationRelay(AbstractRelais *newLiberationRelay)
{
    setRelayHelper(newLiberationRelay, mLiberationRelay);
}

void BEMPanelItem::setRelayHelper(AbstractRelais *newRelay, AbstractRelais *&mRelay)
{
    if(mRelay == newRelay)
        return;

    if(mRelay)
    {
        disconnect(mRelay, &AbstractRelais::stateChanged,
                   this, &BEMPanelItem::triggerUpdate);
        disconnect(mRelay, &AbstractRelais::settingsChanged,
                   this, &BEMPanelItem::triggerUpdate);
        disconnect(mRelay, &AbstractRelais::destroyed,
                   this, &BEMPanelItem::onRelayDestroyed);
    }

    mRelay = newRelay;

    if(mRelay)
    {
        connect(mRelay, &AbstractRelais::stateChanged,
                this, &BEMPanelItem::triggerUpdate);
        connect(mRelay, &AbstractRelais::settingsChanged,
                this, &BEMPanelItem::triggerUpdate);
        connect(mRelay, &AbstractRelais::destroyed,
                this, &BEMPanelItem::onRelayDestroyed);
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit settingsChanged();
}

AbstractRelais *BEMPanelItem::getC1Relay() const
{
    return mC1Relay;
}

void BEMPanelItem::setC1Relay(AbstractRelais *newC1Relay)
{
    setRelayHelper(newC1Relay, mC1Relay);
}

AbstractRelais *BEMPanelItem::getR1Relay() const
{
    return mR1Relay;
}

void BEMPanelItem::setR1Relay(AbstractRelais *newR1Relay)
{
    setRelayHelper(newR1Relay, mR1Relay);
}

AbstractRelais *BEMPanelItem::getKConditionsRelay() const
{
    return mKConditionsRelay;
}

void BEMPanelItem::setKConditionsRelay(AbstractRelais *newKConditionsRelay)
{
    setRelayHelper(newKConditionsRelay, mKConditionsRelay);
}

AbstractRelais *BEMPanelItem::getOccupancyRelay() const
{
    return mOccupancyRelay;
}

void BEMPanelItem::setOccupancyRelay(AbstractRelais *newOccupancyRelay)
{
    setRelayHelper(newOccupancyRelay, mOccupancyRelay);
}

void BEMPanelItem::setArtLibButton(AbstractSimulationObject *newButton)
{
    ButtonInterface *butIface = newButton ? newButton->getInterface<ButtonInterface>() : nullptr;

    if(butIface == mArtificialLibBut)
        return;

    if(mArtificialLibBut)
    {
        AbstractSimulationObject *oldBut = mArtificialLibBut->object();

        disconnect(oldBut, &AbstractSimulationObject::stateChanged,
                   this, &BEMPanelItem::triggerUpdate);
        disconnect(oldBut, &AbstractSimulationObject::settingsChanged,
                   this, &BEMPanelItem::triggerUpdate);
        mArtificialLibBut = nullptr;
    }

    if(newButton && butIface)
    {
        connect(newButton, &AbstractSimulationObject::stateChanged,
                this, &BEMPanelItem::triggerUpdate);
        connect(newButton, &AbstractSimulationObject::settingsChanged,
                this, &BEMPanelItem::triggerUpdate);

        mArtificialLibBut = butIface;
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    //emit buttonChanged(mButton);
}

void BEMPanelItem::onConsLeverDestroyed()
{
    setConsensusLever(nullptr);
}

bool distanceLess(const QPointF& diff, double radius)
{
    // Add some tolerance
    radius *= 1.1;
    radius += 1;

    // Pitagora
    return (diff.x() * diff.x() + diff.y() * diff.y()) < (radius * radius);
}

void BEMPanelItem::onConsLeverInterfaceChanged(const QString &ifaceName, const QString &propName, const QVariant &value)
{
    if(ifaceName == BEMHandleInterface::IfaceType)
    {
        if(!mConsLeverObj)
            return;

        BEMHandleInterface *consLeverIface = mConsLeverObj->getInterface<BEMHandleInterface>();


        if(propName == BEMHandleInterface::TwinLeverPropName)
        {
            BEMHandleInterface *reqLever = consLeverIface->getTwinHandle();
            setRequestLever(reqLever ? static_cast<BEMLeverObject *>(reqLever->object()) : nullptr);
        }
        else if(propName == BEMHandleInterface::LeverTypePropName)
        {
            if(consLeverIface->leverType() == BEMHandleInterface::LeverType::Request)
            {
                // Unset lever
                setConsensusLever(nullptr);

                if(BEMHandleInterface *newConsLever = consLeverIface->getTwinHandle())
                {
                    // Swap levers
                    setConsensusLever(static_cast<BEMLeverObject *>(newConsLever->object()));
                }
            }
        }
        else if(propName == BEMHandleInterface::ArtificialLibButPropName)
        {
            ButtonInterface *artLibBut = consLeverIface->artificialLiberation();
            setArtLibButton(artLibBut ? artLibBut->object() : nullptr);
        }
        else if(propName == BEMHandleInterface::LibRelayPropName)
        {
            setArtLibButton(consLeverIface->liberationRelay());
        }
    }
}

void BEMPanelItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing
            && ev->button() == Qt::LeftButton)
    {
        bool shouldAccept = true;

        if(distanceLess(ev->pos() - LightButtonCenter, innerSmallRadius))
        {
            mMouseState = MouseState::LightButton;
            if(mLightButton)
                mLightButton->setState(ButtonInterface::State::Pressed);
        }
        else if(distanceLess(ev->pos() - ArtLibCenter, artificialLiberationRadius))
        {
            mMouseState = MouseState::ArtificialLibButton;
            if(mArtificialLibBut)
                mArtificialLibBut->setState(ButtonInterface::State::Pressed);
        }
        else if(ev->pos().x() < (ItemWidth / 2) && distanceLess(ev->pos() - ReqLeverCenter, leverRectBaseRadius + leverLength))
        {
            // Request side
            if(distanceLess(ev->pos() - ReqLeverCenter, leverRectBaseRadius))
            {
                // Tx Button 1
                mMouseState = MouseState::TxButton;
                if(mTxButton)
                    mTxButton->setState(ButtonInterface::State::Pressed);
            }
            else
            {
                mMouseState = MouseState::RequestLever;
                if(mReqLever)
                    mReqLever->setPressed(true);
            }
        }
        else if(ev->pos().x() > (ItemWidth / 2) && distanceLess(ev->pos() - ConsLeverCenter, leverRectBaseRadius + leverLength))
        {
            // Consensus side
            if(distanceLess(ev->pos() - ConsLeverCenter, leverRectBaseRadius))
            {
                // Tx Button 2
                mMouseState = MouseState::TxButton;
                if(mTxButton)
                    mTxButton->setState(ButtonInterface::State::Pressed);
            }
            else
            {
                mMouseState = MouseState::ConsensusLever;
                if(mConsLever)
                    mConsLever->setPressed(true);
            }
        }
        else
        {
            shouldAccept = false;
        }

        if(shouldAccept)
        {
            ev->accept();
            mLastMousePos = ev->pos();
            return;
        }
    }

    SnappablePanelItem::mousePressEvent(ev);
}

void BEMPanelItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing)
    {
        QPointF leverCenter;
        LeverInterface *leverInterface = nullptr;

        switch (mMouseState)
        {
        case MouseState::RequestLever:
        {
            leverCenter = ReqLeverCenter;
            leverInterface = mReqLever;
            break;
        }
        case MouseState::ConsensusLever:
        {
            leverCenter = ConsLeverCenter;
            leverInterface = mConsLever;
            break;
        }
        default:
            break;
        }

        if(leverInterface && ev->buttons() & Qt::LeftButton)
        {
            QPointF delta = ev->pos() - mLastMousePos;
            if(delta.manhattanLength() > 2)
            {
                // Calculate angle
                delta = leverCenter - ev->pos();

                // Zero is vertical up, so x/y are swapped
                // Also returned angle must be inverted to be clockwise
                const double angleRadiants = -qAtan2(delta.x(), delta.y());

                int newAngle = qRound(qRadiansToDegrees(angleRadiants));

                // Disable snap with shift
                if(ev->modifiers() == Qt::ShiftModifier)
                    leverInterface->setAngle(newAngle);
                else
                    leverInterface->setAngleTrySnap(newAngle);

                mLastMousePos = ev->pos();

                ev->accept();
                return;
            }
        }
    }

    SnappablePanelItem::mouseMoveEvent(ev);
}

void BEMPanelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing)
    {
        // We don't care about button
        // Also sometimes there are already no buttons
        switch (mMouseState)
        {
        case MouseState::TxButton:
        {
            if(mTxButton && mTxButton->mode() == ButtonInterface::Mode::ReturnNormalOnRelease)
            {
                mTxButton->setState(ButtonInterface::State::Normal);
            }
            break;
        }
        case MouseState::ArtificialLibButton:
        {
            if(mArtificialLibBut && mArtificialLibBut->mode() == ButtonInterface::Mode::ReturnNormalOnRelease)
            {
                mArtificialLibBut->setState(ButtonInterface::State::Normal);
            }
            break;
        }
        case MouseState::LightButton:
        {
            if(mLightButton && mLightButton->mode() == ButtonInterface::Mode::ReturnNormalOnRelease)
            {
                mLightButton->setState(ButtonInterface::State::Normal);
            }
            break;
        }
        case MouseState::RequestLever:
        {
            if(mReqLever)
            {
                mReqLever->setPressed(false);
            }
            break;
        }
        case MouseState::ConsensusLever:
        {
            if(mConsLever)
            {
                mConsLever->setPressed(false);
            }
            break;
        }
        default:
            break;
        }
    }

    mMouseState = MouseState::None;

    SnappablePanelItem::mouseReleaseEvent(ev);
}
