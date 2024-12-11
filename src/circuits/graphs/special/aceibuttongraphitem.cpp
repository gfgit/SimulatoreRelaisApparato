/**
 * src/circuits/graphs/special/aceibuttongraphitem.cpp
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

#include "aceibuttongraphitem.h"

//TODO: fake
#include "../../nodes/onoffswitchnode.h"
#include <QJsonObject>

#include "../../../objects/abstractsimulationobject.h"
#include "../../../objects/abstractsimulationobjectmodel.h"

#include "../../../objects/interfaces/buttoninterface.h"

#include "../../../objects/simple_activable/lightbulbobject.h"

#include "../../../views/modemanager.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

ACEIButtonGraphItem::ACEIButtonGraphItem(OnOffSwitchNode *node_)
    : AbstractNodeGraphItem(node_)
{

}

void ACEIButtonGraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractNodeGraphItem::paint(painter, option, widget);

    constexpr QPointF center(TileLocation::HalfSize,
                             TileLocation::HalfSize);

    constexpr QPointF buttonCenter(center.x(), center.y() + 5);

    constexpr double baseCircleRadius = 24;
    constexpr double buttonCircleRadius = 16;

    constexpr double lightCircleRadius = 12;
    constexpr double lightOffset = 16;

    // Draw dark gray border around
    QPen borderPen;
    borderPen.setWidth(3);
    borderPen.setColor(Qt::darkGray);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());

    // Draw lights
    QRectF circle;
    circle.setSize(QSizeF(lightCircleRadius * 2,
                          lightCircleRadius * 2));

    if(mCentralLight)
    {
        // White light (We draw yellow to have contrast)
        // and because incandescent light bulb are never white
        if(mCentralLight->state() == LightBulbObject::State::On)
            painter->setBrush(Qt::yellow);
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(QPointF(buttonCenter.x(), lightOffset));
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
    textRect.setRight(TileLocation::Size - 10.0);
    textRect.setTop(TileLocation::HalfSize + 20.0);
    textRect.setBottom(TileLocation::Size - 4.0);

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

void ACEIButtonGraphItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(getAbstractNode()->modeMgr()->mode() != FileMode::Editing
            && mButtonIface && boundingRect().contains(ev->pos()))
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

    AbstractNodeGraphItem::mousePressEvent(ev);
}

void ACEIButtonGraphItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if(getAbstractNode()->modeMgr()->mode() != FileMode::Editing
            && mButtonIface)
    {
        // We don't care about mouse button
        // Also sometimes there are already no buttons
        if(mButtonIface->mode() == ButtonInterface::Mode::ReturnNormalOnRelease)
            mButtonIface->setState(ButtonInterface::State::Normal);
        return;
    }

    AbstractNodeGraphItem::mouseReleaseEvent(ev);
}

LightBulbObject *ACEIButtonGraphItem::centralLight() const
{
    return mCentralLight;
}

void ACEIButtonGraphItem::setCentralLight(LightBulbObject *newCentralLight)
{
    if(mCentralLight == newCentralLight)
        return;

    if(mCentralLight)
    {
        disconnect(mCentralLight, &LightBulbObject::stateChanged,
                   this, &ACEIButtonGraphItem::triggerUpdate);
        disconnect(mCentralLight, &LightBulbObject::destroyed,
                   this, &ACEIButtonGraphItem::onLightDestroyed);
    }

    mCentralLight = newCentralLight;

    if(mCentralLight)
    {
        connect(mCentralLight, &LightBulbObject::stateChanged,
                this, &ACEIButtonGraphItem::triggerUpdate);
        connect(mCentralLight, &LightBulbObject::destroyed,
                this, &ACEIButtonGraphItem::onLightDestroyed);
    }

    getAbstractNode()->modeMgr()->setFileEdited();
    update();
    emit lightsChanged();
}

AbstractSimulationObject *ACEIButtonGraphItem::button() const
{
    return mButton;
}

void ACEIButtonGraphItem::setButton(AbstractSimulationObject *newButton)
{
    if(newButton && !newButton->getInterface<ButtonInterface>())
        return;

    if(mButton)
    {
        disconnect(mButton, &AbstractSimulationObject::destroyed,
                   this, &ACEIButtonGraphItem::onButtonDestroyed);
        disconnect(mButton, &AbstractSimulationObject::stateChanged,
                   this, &ACEIButtonGraphItem::triggerUpdate);
        disconnect(mButton, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ACEIButtonGraphItem::onInterfacePropertyChanged);
        disconnect(mButton, &AbstractSimulationObject::settingsChanged,
                   this, &ACEIButtonGraphItem::triggerUpdate);
        mButtonIface = nullptr;
    }

    mButton = newButton;

    if(mButton)
    {
        connect(mButton, &AbstractSimulationObject::destroyed,
                this, &ACEIButtonGraphItem::onButtonDestroyed);
        connect(mButton, &AbstractSimulationObject::stateChanged,
                this, &ACEIButtonGraphItem::triggerUpdate);
        connect(mButton, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ACEIButtonGraphItem::onInterfacePropertyChanged);
        connect(mButton, &AbstractSimulationObject::settingsChanged,
                this, &ACEIButtonGraphItem::triggerUpdate);

        mButtonIface = mButton->getInterface<ButtonInterface>();
    }

    emit buttonChanged(mButton);
}

bool ACEIButtonGraphItem::loadFromJSON(const QJsonObject &obj)
{
    QJsonObject objCopy = obj;

    // Restore fake node type
    objCopy["type"] = Node::NodeType;

    const QString buttonName = obj.value("button").toString();
    const QString buttonType = obj.value("button_type").toString();
    auto model = getAbstractNode()->modeMgr()->modelForType(buttonType);

    if(model)
        setButton(model->getObjectByName(buttonName));
    else
        setButton(nullptr);

    auto lightModel = getAbstractNode()->modeMgr()->modelForType(LightBulbObject::Type);
    if(lightModel)
    {
        const QString centralLightName = obj.value("light_central").toString();
        setCentralLight(static_cast<LightBulbObject *>(lightModel->getObjectByName(centralLightName)));
    }
    else
    {
        setCentralLight(nullptr);
    }

    return AbstractNodeGraphItem::loadFromJSON(objCopy);
}

void ACEIButtonGraphItem::saveToJSON(QJsonObject &obj) const
{
    AbstractNodeGraphItem::saveToJSON(obj);

    // Replace fake node type with ours
    obj["type"] = CustomNodeType;

    obj["button"] = mButton ? mButton->name() : QString();
    obj["button_type"] = mButton ? mButton->getType() : QString();

    obj["light_central"] = mCentralLight ? mCentralLight->name() : QString();
}

void ACEIButtonGraphItem::onButtonDestroyed()
{
    mButton = nullptr;
    mButtonIface = nullptr;
    emit buttonChanged(mButton);
}

void ACEIButtonGraphItem::onLightDestroyed()
{
    setCentralLight(nullptr);
}

void ACEIButtonGraphItem::onInterfacePropertyChanged(const QString &ifaceName, const QString &propName, const QVariant &value)
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

QString FakeACEIButtonNode::nodeType() const
{
    return FakeACEIButtonNode::NodeType;
}

