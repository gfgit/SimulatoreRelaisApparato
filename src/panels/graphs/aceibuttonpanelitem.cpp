/**
 * src/panels/graphs/aceibuttonpanelitem.cpp
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

#include "aceibuttonpanelitem.h"
#include "../panelscene.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../objects/interfaces/buttoninterface.h"

#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../views/modemanager.h"

#include "../../utils/enum_desc.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

#include <QJsonObject>

ACEIButtonPanelItem::ACEIButtonPanelItem()
    : SnappablePanelItem()
{

}

ACEIButtonPanelItem::~ACEIButtonPanelItem()
{
    for(int i = 0; i < NLights; i++)
    {
        setLight(LightPosition(i), nullptr);
    }

    setButton(nullptr);
}

QString ACEIButtonPanelItem::itemType() const
{
    return ItemType;
}

QString ACEIButtonPanelItem::tooltipString() const
{
    if(!mButtonIface)
    {
        return tr("NO BUTTON SET!!!");
    }

    QString butPosName = ButtonInterface::getStateDesc().name(int(mButtonIface->state()));

    QString tipText = tr("Button <b>%1</b><br>"
                         "State: %2").arg(mButton->name(),
                                          butPosName);
    if(!mButton->description().isEmpty())
    {
        tipText.append(QLatin1String("<br><br>"));
        tipText.append(mButton->description().toHtmlEscaped());
    }

    return tipText;
}

QRectF ACEIButtonPanelItem::boundingRect() const
{
    return QRectF(0, 0, ItemWidth, ItemHeight);
}

void ACEIButtonPanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QRectF br = boundingRect();

    // Background
    painter->fillRect(br, isSelected() ? SelectedBackground : qRgb(0x7F, 0x7F, 0x7F));

    const QPointF center = br.center();

    const QPointF buttonCenter(center.x(), center.y());

    constexpr QRgb BorderColor = qRgb(97, 97, 97);

    // Draw dark gray border around
    QPen borderPen;
    borderPen.setWidth(2);
    borderPen.setColor(BorderColor);
    borderPen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));

    borderPen.setWidth(3);
    painter->setPen(borderPen);

    // Draw lights
    QRectF circle;
    circle.setSize(QSizeF(lightCircleRadius * 2,
                          lightCircleRadius * 2));

    if(LightBulbObject *leftLight = getLight(LightPosition::Left))
    {
        if(leftLight->state() == LightBulbObject::State::On)
            painter->setBrush(getLightColor(LightPosition::Left));
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(lightOffsetX, lightOffsetY));
        painter->drawEllipse(circle);
    }

    if(LightBulbObject *centralLight = getLight(LightPosition::Central))
    {
        if(centralLight->state() == LightBulbObject::State::On)
            painter->setBrush(getLightColor(LightPosition::Central));
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(center.x(), lightOffsetCentralY));
        painter->drawEllipse(circle);
    }

    if(LightBulbObject *rightLight = getLight(LightPosition::Right))
    {
        if(rightLight->state() == LightBulbObject::State::On)
            painter->setBrush(getLightColor(LightPosition::Right));
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(br.width() - lightOffsetX, lightOffsetY));
        painter->drawEllipse(circle);
    }

    // Draw base circle below lever
    circle.setSize(QSizeF(baseCircleRadius * 2,
                          baseCircleRadius * 2));
    circle.moveCenter(buttonCenter);
    painter->setBrush(Qt::lightGray);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle);

    // Draw button
    QColor color = qRgb(77, 77, 77); // Dark gray
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);

    ButtonInterface::State state = mButtonIface ?
                mButtonIface->state() :
                ButtonInterface::State::Normal;

    QPointF buttonTopCenter = buttonCenter;
    int buttonRadius = buttonCircleRadius;
    if(state == ButtonInterface::State::Pressed)
    {
        buttonTopCenter.ry() += 1;
        buttonRadius -= 3;
    }
    else if(state == ButtonInterface::State::Extracted)
    {
        buttonTopCenter.ry() -= 1;
        buttonRadius += 3;
    }

    circle.setSize(QSizeF(buttonRadius * 2,
                          buttonRadius * 2));
    circle.moveCenter(buttonTopCenter);
    painter->drawEllipse(circle);

    // Draw Button name
    const QString buttonName = mButton ? mButton->name() : tr("NULL");

    QRectF textRect;
    textRect.setLeft(10);
    textRect.setRight(br.width() - 10.0);
    textRect.setTop(buttonCenter.y() + baseCircleRadius);
    textRect.setBottom(br.bottom() - 4.0);

    Qt::Alignment textAlign = Qt::AlignLeft;

    QFont f;
    f.setPointSizeF(18.0);
    f.setBold(true);

    QFontMetrics metrics(f, painter->device());
    double width = metrics.horizontalAdvance(buttonName, QTextOption(textAlign));
    if(width > textRect.width())
    {
        f.setPointSizeF(f.pointSizeF() * textRect.width() / width * 0.9);
    }

    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::black);

    painter->setFont(f);
    painter->drawText(textRect, textAlign, buttonName);
}

void ACEIButtonPanelItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing
            && mButtonIface
            && boundingRect().contains(ev->pos()))
    {
        ButtonInterface::State state = mButtonIface->state();

        if(ev->button() == Qt::LeftButton)
        {
            // Go down by one
            switch (state)
            {
            case ButtonInterface::State::Extracted:
                state = ButtonInterface::State::Normal;
                break;
            case ButtonInterface::State::Normal:
                state = ButtonInterface::State::Pressed;
                break;
            default:
                return; // Cannot go lower
            }
        }
        else if(ev->button() == Qt::RightButton)
        {
            // Go down by one
            switch (state)
            {
            case ButtonInterface::State::Pressed:
                state = ButtonInterface::State::Normal;
                break;
            case ButtonInterface::State::Normal:
                state = ButtonInterface::State::Extracted;
                break;
            default:
                return; // Cannot go higher
            }
        }

        mButtonIface->setState(state);
        return;
    }

    SnappablePanelItem::mousePressEvent(ev);
}

void ACEIButtonPanelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing
            && mButtonIface)
    {
        // We don't care about mouse button
        // Also sometimes there are already no buttons
        if(mButtonIface->mode() == ButtonInterface::Mode::ReturnNormalOnRelease)
            mButtonIface->setState(ButtonInterface::State::Normal);
        return;
    }

    SnappablePanelItem::mouseReleaseEvent(ev);
}

AbstractSimulationObject *ACEIButtonPanelItem::button() const
{
    return mButton;
}

void ACEIButtonPanelItem::setButton(AbstractSimulationObject *newButton)
{
    if(newButton && !newButton->getInterface<ButtonInterface>())
        return;

    if(mButton)
    {
        disconnect(mButton, &AbstractSimulationObject::destroyed,
                   this, &ACEIButtonPanelItem::onButtonDestroyed);
        disconnect(mButton, &AbstractSimulationObject::stateChanged,
                   this, &ACEIButtonPanelItem::triggerUpdate);
        disconnect(mButton, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ACEIButtonPanelItem::onInterfacePropertyChanged);
        disconnect(mButton, &AbstractSimulationObject::settingsChanged,
                   this, &ACEIButtonPanelItem::triggerUpdate);
        mButtonIface = nullptr;
    }

    mButton = newButton;

    if(mButton)
    {
        connect(mButton, &AbstractSimulationObject::destroyed,
                this, &ACEIButtonPanelItem::onButtonDestroyed);
        connect(mButton, &AbstractSimulationObject::stateChanged,
                this, &ACEIButtonPanelItem::triggerUpdate);
        connect(mButton, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ACEIButtonPanelItem::onInterfacePropertyChanged);
        connect(mButton, &AbstractSimulationObject::settingsChanged,
                this, &ACEIButtonPanelItem::triggerUpdate);

        mButtonIface = mButton->getInterface<ButtonInterface>();
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit buttonChanged(mButton);
}

void ACEIButtonPanelItem::setLight(LightPosition pos, LightBulbObject *newLight)
{
    LightBulbObject *&target = mLights[pos];

    if(target == newLight)
        return;

    if(target)
    {
        disconnect(target, &LightBulbObject::stateChanged,
                   this, &ACEIButtonPanelItem::triggerUpdate);
        disconnect(target, &LightBulbObject::destroyed,
                   this, &ACEIButtonPanelItem::onLightDestroyed);
    }

    target = newLight;

    if(target)
    {
        connect(target, &LightBulbObject::stateChanged,
                this, &ACEIButtonPanelItem::triggerUpdate);
        connect(target, &LightBulbObject::destroyed,
                this, &ACEIButtonPanelItem::onLightDestroyed);
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

void ACEIButtonPanelItem::setLightColor(LightPosition pos, const QColor &newLightColor)
{
    QColor &target = mLightColors[pos];

    if(target == newLightColor)
        return;

    target = newLightColor;

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

bool ACEIButtonPanelItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!SnappablePanelItem::loadFromJSON(obj, mgr))
        return false;

    const QString buttonName = obj.value("button").toString();
    const QString buttonType = obj.value("button_type").toString();
    auto model = mgr->modelForType(buttonType);

    if(model)
        setButton(model->getObjectByName(buttonName));
    else
        setButton(nullptr);

    // Lights
    auto lightModel = mgr->modelForType(LightBulbObject::Type);

    for(int i = 0; i < LightPosition::NLights; i++)
    {
        const QString lightObjName = obj.value(lightFmt.arg(lightKeyNames[i])).toString();
        LightBulbObject *light = nullptr;
        if(lightModel)
            light = static_cast<LightBulbObject *>(lightModel->getObjectByName(lightObjName));

        setLight(LightPosition(i), light);

        QColor c = QColor::fromString(obj.value(lightColorFmt.arg(lightKeyNames[i])).toString());
        if(!c.isValid())
            c = lightDefaultColors[i];

        setLightColor(LightPosition(i), c);
    }

    return true;
}

void ACEIButtonPanelItem::saveToJSON(QJsonObject &obj) const
{
    SnappablePanelItem::saveToJSON(obj);

    obj["button"] = mButton ? mButton->name() : QString();
    obj["button_type"] = mButton ? mButton->getType() : QString();

    // Lights
    for(int i = 0; i < LightPosition::NLights; i++)
    {
        LightBulbObject *light = getLight(LightPosition(i));
        obj[lightFmt.arg(lightKeyNames[i])] = light ? light->name() : QString();
        obj[lightColorFmt.arg(lightKeyNames[i])] = getLightColor(LightPosition(i)).name(QColor::HexRgb);
    }
}

void ACEIButtonPanelItem::onButtonDestroyed()
{
    setButton(nullptr);
}

void ACEIButtonPanelItem::onLightDestroyed()
{
    for(int i = 0; i < NLights; i++)
    {
        if(getLight(LightPosition(i)) == sender())
            setLight(LightPosition(i), nullptr);
    }
}

void ACEIButtonPanelItem::onInterfacePropertyChanged(const QString &ifaceName, const QString &propName, const QVariant &value)
{
    if(ifaceName == ButtonInterface::IfaceType)
    {
        if(!mButtonIface)
            return;

        if(propName == ButtonInterface::StatePropName)
        {
            triggerUpdate();
        }
    }
}
