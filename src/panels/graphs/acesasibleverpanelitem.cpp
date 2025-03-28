/**
 * src/panels/graphs/acesasibleverpanelitem.cpp
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

#include "acesasibleverpanelitem.h"
#include "../panelscene.h"

#include "../../objects/abstractsimulationobject.h"
#include "../../objects/abstractsimulationobjectmodel.h"

#include "../../objects/interfaces/leverinterface.h"
#include "../../objects/interfaces/sasibaceleverextrainterface.h"
#include "../../objects/interfaces/buttoninterface.h"

#include "../../objects/simple_activable/lightbulbobject.h"

#include "../../views/modemanager.h"

#include <QGraphicsSceneMouseEvent>

#include <QPainter>
#include <QPen>

#include <QtMath>

#include <QJsonObject>

ACESasibLeverPanelItem::ACESasibLeverPanelItem()
    : SnappablePanelItem()
{

}

ACESasibLeverPanelItem::~ACESasibLeverPanelItem()
{
    setLever(nullptr);
}

QString ACESasibLeverPanelItem::itemType() const
{
    return ItemType;
}

QString ACESasibLeverPanelItem::tooltipString() const
{
    if(!mLeverIface)
    {
        return tr("NO LEVER SET!!!");
    }

    const int leverPos = mLeverIface->position();
    const auto& desc = mLeverIface->positionDesc();

    QString posStr;
    if(mLeverIface->isPositionMiddle(leverPos))
    {
        // Position index increases going upwards
        // so we say between above position (+1) and below position (-1)
        posStr = tr("Between<br>"
                    "<b>%1</b><br>"
                    "and<br>"
                    "<b>%2</b>")
                .arg(desc.name(leverPos + 1), desc.name(leverPos - 1));
    }
    else
    {
        posStr = tr("<b>%1</b>")
                .arg(desc.name(leverPos));
    }

    QString tipText = tr("ACE Lever: <b>%1</b><br>"
                         "%2")
            .arg(mLever->name(), posStr);

    if(!mLever->description().isEmpty())
    {
        tipText.append(QLatin1String("<br><br>"));
        tipText.append(mLever->description().toHtmlEscaped());
    }

    return tipText;
}

QRectF ACESasibLeverPanelItem::boundingRect() const
{
    return QRectF(0, 0, ItemWidth, ItemHeight);
}

void ACESasibLeverPanelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QRectF br = boundingRect();

    // Background
    painter->fillRect(br, isSelected() ? SelectedBackground : qRgb(0x7F, 0x7F, 0x7F));

    const QPointF center = br.center();

    QRectF hole(QPointF(), holeSize);
    hole.moveCenter(LeverHoleCenter);

    constexpr QRgb BorderColor = qRgb(97, 97, 97);

    // Draw dark gray border around
    QPen borderPen;
    borderPen.setWidth(4);
    borderPen.setColor(BorderColor);
    borderPen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));

    borderPen.setWidth(3);
    painter->setPen(borderPen);

    painter->fillRect(hole, Qt::lightGray);

    constexpr double circleRadius = 25;

    QRectF circle;
    circle.setSize(QSizeF(circleRadius * 2,
                          circleRadius * 2));

    // Angle is vertical (+/- 90) -> cos is null
    // We get 0 for vertical lever and we want it to be 90, so add +90
    const double angleRadiants = qDegreesToRadians(mLeverIface ? mLeverIface->angle() + 90: 0);

    // This is inverted because Y axis increases from top to bottom (goes downwards)
    const double leverY = qCos(angleRadiants) * hole.height() / 2.0;

    const QPointF leverTip(center.x(), leverY + hole.center().y());
    circle.moveCenter(leverTip);

    QColor color = Qt::darkCyan;
    if(!mLeverIface || mLeverIface->isPressed())
        color = Qt::blue;

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setColor(color);
    pen.setWidth(12);

    // Draw lever line
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(hole.center(), leverTip);

    // Draw circle
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(circle);

    // Draw Lever name
    const QString leverName = mLever ? mLever->name() : tr("NULL");

    QRectF textRect;
    textRect.setLeft(10);
    textRect.setRight(br.width() - 10.0);
    textRect.setTop(hole.bottom() + circleRadius);
    textRect.setBottom(244);

    Qt::Alignment textAlign = Qt::AlignCenter;

    QFont f;
    f.setPointSizeF(18.0);
    f.setBold(true);

    QFontMetrics metrics(f, painter->device());
    double width = metrics.horizontalAdvance(leverName, QTextOption(textAlign));
    if(width > textRect.width())
    {
        f.setPointSizeF(f.pointSizeF() * textRect.width() / width * 0.9);
    }

    painter->setBrush(Qt::NoBrush);
    pen.setColor(Qt::black);
    painter->setPen(pen);

    painter->setFont(f);
    painter->drawText(textRect, textAlign, leverName);

    // Draw buttons
    QRectF baseCircle;
    baseCircle.setSize(QSizeF(buttonBaseCircleRadius * 2,
                              buttonBaseCircleRadius * 2));

    if(ButtonInterface *leftBut = mButtons[LightPosition::Left].buttonIface)
    {
        ButtonInterface::State state = leftBut ?
                    leftBut->state() :
                    ButtonInterface::State::Normal;

        baseCircle.moveCenter(LeftButCenter);
        QPointF buttonTopCenter = LeftButCenter;

        int buttonRadius = buttonCircleRadius;
        if(state == ButtonInterface::State::Pressed)
        {
            buttonTopCenter.ry() += 1;
            buttonRadius -= 3;
            f.setPointSizeF(13);
        }
        else if(state == ButtonInterface::State::Extracted)
        {
            buttonTopCenter.ry() -= 1;
            buttonRadius += 3;
            f.setPointSizeF(17);
        }
        else
        {
            f.setPointSizeF(15);
        }

        // Draw base circle below lever
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(baseCircle);

        // Draw button
        painter->setBrush(Qt::lightGray);
        painter->setPen(borderPen);

        circle.setSize(QSizeF(buttonRadius * 2,
                              buttonRadius * 2));
        circle.moveCenter(buttonTopCenter);
        painter->drawEllipse(circle);

        // Draw button name (Up to 3 characters or first space)
        const QString butName = leftBut->object()->name();
        const qsizetype spaceIdx = butName.indexOf(' ');
        qsizetype length = 3;
        if(spaceIdx >= 0 && spaceIdx < length)
            length = spaceIdx;

        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);

        painter->setFont(f);
        painter->drawText(circle, textAlign, butName.left(length));
    }

    if(ButtonInterface *rightBut = mButtons[LightPosition::Right].buttonIface)
    {
        ButtonInterface::State state = rightBut ?
                    rightBut->state() :
                    ButtonInterface::State::Normal;

        baseCircle.moveCenter(RightButCenter);
        QPointF buttonTopCenter = RightButCenter;

        int buttonRadius = buttonCircleRadius;
        if(state == ButtonInterface::State::Pressed)
        {
            buttonTopCenter.ry() += 1;
            buttonRadius -= 3;
            f.setPointSizeF(13);
        }
        else if(state == ButtonInterface::State::Extracted)
        {
            buttonTopCenter.ry() -= 1;
            buttonRadius += 3;
            f.setPointSizeF(17);
        }
        else
        {
            f.setPointSizeF(15);
        }

        // Draw base circle below lever
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(baseCircle);

        // Draw button
        painter->setBrush(Qt::lightGray);
        painter->setPen(borderPen);

        circle.setSize(QSizeF(buttonRadius * 2,
                              buttonRadius * 2));
        circle.moveCenter(buttonTopCenter);
        painter->drawEllipse(circle);

        // Draw button name (Up to 3 characters or first space)
        const QString butName = rightBut->object()->name();
        const qsizetype spaceIdx = butName.indexOf(' ');
        qsizetype length = 3;
        if(spaceIdx >= 0 && spaceIdx < length)
            length = spaceIdx;

        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);

        painter->setFont(f);
        painter->drawText(circle, textAlign, butName.left(length));
    }

    // Draw lights
    circle.setSize(QSizeF(lightCircleRadius * 2,
                          lightCircleRadius * 2));
    painter->setPen(borderPen);

    if(LightBulbObject *leftLight = getLight(LightPosition::Left))
    {
        if(leftLight->state() == LightBulbObject::State::On)
            painter->setBrush(getLightColor(LightPosition::Left));
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(LeftLightCenter);
        painter->drawEllipse(circle);
    }

    if(LightBulbObject *rightLight = getLight(LightPosition::Right))
    {
        if(rightLight->state() == LightBulbObject::State::On)
            painter->setBrush(getLightColor(LightPosition::Right));
        else
            painter->setBrush(Qt::NoBrush);

        circle.moveCenter(RightLightCenter);
        painter->drawEllipse(circle);
    }
}

inline bool distanceLess(const QPointF& diff, double radius)
{
    // Add some tolerance
    radius *= 1.1;
    radius += 1;

    // Pitagora
    return (diff.x() * diff.x() + diff.y() * diff.y()) < (radius * radius);
}

inline bool distanceLessY(const QPointF& diff, double radius)
{
    // Add some tolerance
    radius *= 1.1;
    radius += 1;

    return std::abs(diff.y()) < (radius);
}

void ACESasibLeverPanelItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing)
    {
        bool shouldAccept = true;

        QRectF holeRect;
        holeRect.setSize(holeSize);
        holeRect.moveCenter(LeverHoleCenter);
        holeRect.adjust(-1, -holeSize.width(),
                        +1, holeSize.width());

        if(distanceLess(ev->pos() - LeftButCenter, buttonBaseCircleRadius))
        {
            mMouseState = MouseState::LeftButton;
            if(auto butIface = mButtons[LightPosition::Left].buttonIface)
            {
                if(ev->button() == Qt::LeftButton)
                    butIface->setState(ButtonInterface::State::Pressed);
                else if(ev->button() == Qt::RightButton)
                    butIface->setState(ButtonInterface::State::Extracted);
            }
        }
        else if(distanceLess(ev->pos() - RightButCenter, buttonBaseCircleRadius))
        {
            mMouseState = MouseState::RightButton;
            if(auto butIface = mButtons[LightPosition::Right].buttonIface)
            {
                if(ev->button() == Qt::LeftButton)
                    butIface->setState(ButtonInterface::State::Pressed);
                else if(ev->button() == Qt::RightButton)
                    butIface->setState(ButtonInterface::State::Extracted);
            }
        }
        else if(holeRect.contains(ev->pos()))
        {
            // Lever
            mMouseState = MouseState::Lever;
            if(mLeverIface)
                mLeverIface->setPressed(true);
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

void ACESasibLeverPanelItem::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing)
    {
        if(mMouseState == MouseState::Lever &&
                ev->buttons() & Qt::LeftButton)
        {
            QPointF delta = ev->pos() - mLastMousePos;
            if(delta.manhattanLength() > 2)
            {
                QRectF hole(QPointF(), holeSize);
                hole.moveCenter(QPointF(LeverHoleCenter));

                // Calculate angle
                const double leverY = ev->pos().y() - hole.center().y();

                int newAngle = 0;

                // Bound qAcos argument to (0, 1) interval
                if(ev->pos().y() > hole.bottom() || qFuzzyCompare(ev->pos().y(), hole.bottom()))
                    newAngle = -90; // Below bottom
                else if(ev->pos().y() < hole.top() || qFuzzyCompare(ev->pos().y(), hole.top()))
                    newAngle = 90; // Above top
                else
                {
                    // This is inverted because Y axis increases from top to bottom (goes downwards)
                    const double angleRadiants = qAcos(2.0 * leverY / hole.height());

                    // Angle is vertical (+/- 90)-> cos is null
                    // We get 90 for vertical but we want it to be zero for lever position, so add +90
                    newAngle = qRound(qRadiansToDegrees(angleRadiants)) - 90;
                }

                // Disable snap with shift
                if(ev->modifiers() == Qt::ShiftModifier)
                    mLeverIface->setAngle(newAngle);
                else
                    mLeverIface->setAngleTrySnap(newAngle);

                mLastMousePos = ev->pos();

                ev->accept();
                return;
            }
        }
    }

    SnappablePanelItem::mouseMoveEvent(ev);
}

void ACESasibLeverPanelItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    PanelScene *s = panelScene();
    if(s && s->modeMgr()->mode() != FileMode::Editing)
    {
        // We don't care about button
        // Also sometimes there are already no buttons
        switch (mMouseState)
        {
        case MouseState::LeftButton:
        {
            if(auto butIface = mButtons[LightPosition::Left].buttonIface)
            {
                if(butIface->mode() == ButtonInterface::Mode::ReturnNormalOnRelease)
                    butIface->setState(ButtonInterface::State::Normal);
            }
            break;
        }
        case MouseState::RightButton:
        {
            if(auto butIface = mButtons[LightPosition::Right].buttonIface)
            {
                if(butIface->mode() == ButtonInterface::Mode::ReturnNormalOnRelease)
                    butIface->setState(ButtonInterface::State::Normal);
            }
            break;
        }
        case MouseState::Lever:
        {
            if(mLeverIface)
            {
                mLeverIface->setPressed(false);
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

AbstractSimulationObject *ACESasibLeverPanelItem::lever() const
{
    return mLever;
}

void ACESasibLeverPanelItem::setLever(AbstractSimulationObject *newLever)
{
    if(newLever && !newLever->getInterface<SasibACELeverExtraInterface>())
        return;

    if(mLever)
    {
        disconnect(mLever, &AbstractSimulationObject::destroyed,
                   this, &ACESasibLeverPanelItem::onLeverDestroyed);
        disconnect(mLever, &AbstractSimulationObject::stateChanged,
                   this, &ACESasibLeverPanelItem::triggerUpdate);
        disconnect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ACESasibLeverPanelItem::onInterfacePropertyChanged);
        disconnect(mLever, &AbstractSimulationObject::settingsChanged,
                   this, &ACESasibLeverPanelItem::triggerUpdate);
        mLeverIface = nullptr;
    }

    mLever = newLever;

    if(mLever)
    {
        connect(mLever, &AbstractSimulationObject::destroyed,
                this, &ACESasibLeverPanelItem::onLeverDestroyed);
        connect(mLever, &AbstractSimulationObject::stateChanged,
                this, &ACESasibLeverPanelItem::triggerUpdate);
        connect(mLever, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ACESasibLeverPanelItem::onInterfacePropertyChanged);
        connect(mLever, &AbstractSimulationObject::settingsChanged,
                this, &ACESasibLeverPanelItem::triggerUpdate);

        mLeverIface = mLever->getInterface<LeverInterface>();
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    emit leverChanged(mLever);
}

void ACESasibLeverPanelItem::setButton(LightPosition pos, AbstractSimulationObject *newButton)
{
    if(newButton && !newButton->getInterface<ButtonInterface>())
        return;

    AbstractSimulationObject *&target = mButtons[pos].button;

    if(target == newButton)
        return;

    if(target)
    {
        disconnect(target, &AbstractSimulationObject::destroyed,
                   this, &ACESasibLeverPanelItem::onButtonDestroyed);
        disconnect(target, &AbstractSimulationObject::stateChanged,
                   this, &ACESasibLeverPanelItem::triggerUpdate);
        disconnect(target, &AbstractSimulationObject::interfacePropertyChanged,
                   this, &ACESasibLeverPanelItem::onInterfacePropertyChanged);
        disconnect(target, &AbstractSimulationObject::settingsChanged,
                   this, &ACESasibLeverPanelItem::triggerUpdate);

        mButtons[pos].buttonIface = nullptr;
    }

    target = newButton;

    if(target)
    {
        connect(target, &AbstractSimulationObject::destroyed,
                this, &ACESasibLeverPanelItem::onButtonDestroyed);
        connect(target, &AbstractSimulationObject::stateChanged,
                this, &ACESasibLeverPanelItem::triggerUpdate);
        connect(target, &AbstractSimulationObject::interfacePropertyChanged,
                this, &ACESasibLeverPanelItem::onInterfacePropertyChanged);
        connect(target, &AbstractSimulationObject::settingsChanged,
                this, &ACESasibLeverPanelItem::triggerUpdate);

        mButtons[pos].buttonIface = target->getInterface<ButtonInterface>();
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit buttonsChanged();
}

void ACESasibLeverPanelItem::setLight(LightPosition pos, LightBulbObject *newLight)
{
    LightBulbObject *&target = mLights[pos];

    if(target == newLight)
        return;

    if(target)
    {
        disconnect(target, &LightBulbObject::stateChanged,
                   this, &ACESasibLeverPanelItem::triggerUpdate);
        disconnect(target, &LightBulbObject::destroyed,
                   this, &ACESasibLeverPanelItem::onLightDestroyed);
    }

    target = newLight;

    if(target)
    {
        connect(target, &LightBulbObject::stateChanged,
                this, &ACESasibLeverPanelItem::triggerUpdate);
        connect(target, &LightBulbObject::destroyed,
                this, &ACESasibLeverPanelItem::onLightDestroyed);
    }

    PanelScene *s = panelScene();
    if(s)
        s->modeMgr()->setFileEdited();

    update();
    emit lightsChanged();
}

void ACESasibLeverPanelItem::setLightColor(LightPosition pos, const QColor &newLightColor)
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

bool ACESasibLeverPanelItem::loadFromJSON(const QJsonObject &obj, ModeManager *mgr)
{
    if(!SnappablePanelItem::loadFromJSON(obj, mgr))
        return false;

    // Lever
    const QString leverName = obj.value("lever").toString();
    const QString leverType = obj.value("lever_type").toString();
    auto model = mgr->modelForType(leverType);

    if(model)
        setLever(model->getObjectByName(leverName));
    else
        setLever(nullptr);

    // Buttons
    for(int i = 0; i < LightPosition::NLights; i++)
    {
        const QString butName = obj.value(butFmt.arg(lightKeyNames[i])).toString();
        const QString butType = obj.value(butTypeFmt.arg(lightKeyNames[i])).toString();

        auto butModel = mgr->modelForType(butType);
        if(!butModel)
            continue;

        AbstractSimulationObject *butObj = butModel->getObjectByName(butName);
        setButton(LightPosition(i), butObj);
    }

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

void ACESasibLeverPanelItem::saveToJSON(QJsonObject &obj) const
{
    SnappablePanelItem::saveToJSON(obj);

    // Lever
    obj["lever"] = mLever ? mLever->name() : QString();
    obj["lever_type"] = mLever ? mLever->getType() : QString();

    // Buttons
    for(int i = 0; i < LightPosition::NLights; i++)
    {
        AbstractSimulationObject *butObj = getButton(LightPosition(i));
        obj[butFmt.arg(lightKeyNames[i])] = butObj ? butObj->name() : QString();
        obj[butTypeFmt.arg(lightKeyNames[i])] = butObj ? butObj->getType() : QString();
    }

    // Lights
    for(int i = 0; i < LightPosition::NLights; i++)
    {
        LightBulbObject *light = getLight(LightPosition(i));
        obj[lightFmt.arg(lightKeyNames[i])] = light ? light->name() : QString();
        obj[lightColorFmt.arg(lightKeyNames[i])] = getLightColor(LightPosition(i)).name(QColor::HexRgb);
    }
}

void ACESasibLeverPanelItem::getObjectProperties(QVector<ObjectProperty> &result) const
{
    ObjectProperty leverProp;
    leverProp.name = "lever";
    leverProp.prettyName = tr("Lever");
    leverProp.interface = SasibACELeverExtraInterface::IfaceType;
    result.append(leverProp);

    for(int i = 0; i < LightPosition::NLights; i++)
    {
        ObjectProperty butProp;
        butProp.name = butFmt.arg(lightKeyNames[i]);
        butProp.prettyName = tr("Button %1").arg(i);
        butProp.customType = butTypeFmt.arg(lightKeyNames[i]);
        butProp.interface = ButtonInterface::IfaceType;
        result.append(butProp);
    }

    for(int i = 0; i < LightPosition::NLights; i++)
    {
        ObjectProperty lightProp;
        lightProp.name = lightFmt.arg(lightKeyNames[i]);
        lightProp.prettyName = tr("Light %1").arg(i);
        lightProp.types = {LightBulbObject::Type};
        result.append(lightProp);
    }
}

void ACESasibLeverPanelItem::onLeverDestroyed()
{
    setLever(nullptr);
}

void ACESasibLeverPanelItem::onButtonDestroyed(QObject *obj)
{
    for(int i = 0; i < NLights; i++)
    {
        if(getButton(LightPosition(i)) == obj)
            setButton(LightPosition(i), nullptr);
    }
}

void ACESasibLeverPanelItem::onLightDestroyed(QObject *obj)
{
    for(int i = 0; i < NLights; i++)
    {
        if(getLight(LightPosition(i)) == obj)
            setLight(LightPosition(i), nullptr);
    }
}

void ACESasibLeverPanelItem::onInterfacePropertyChanged(const QString &ifaceName, const QString &propName, const QVariant &value)
{
    if(ifaceName == LeverInterface::IfaceType)
    {
        if(!mLeverIface)
            return;
    }
    if(ifaceName == ButtonInterface::IfaceType)
    {
        if(propName == ButtonInterface::StatePropName)
        {
            triggerUpdate();
        }
    }
}
