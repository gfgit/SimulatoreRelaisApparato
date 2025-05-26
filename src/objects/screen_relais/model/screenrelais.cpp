/**
 * src/objects/screen_relais/model/screenrelais.cpp
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

#include "screenrelais.h"

#include "../../../circuits/nodes/screenrelaiscontactnode.h"
#include "../../../circuits/nodes/screenrelaispowernode.h"

#include "../../../utils/enum_desc.h"

#include <QTimerEvent>

#include <QJsonObject>
#include <QCborMap>

#include <QRandomGenerator>

QString ScreenRelais::getScreenTypeName(ScreenType t)
{
    switch(t)
    {
    case ScreenType::CenteredScreen:
        return tr("Centered Screen");
    case ScreenType::DecenteredScreen:
        return tr("Decentered Screen");
    case ScreenType::NTypes:
        break;
    }

    return QString();
}

static const EnumDesc screen_type_desc =
{
    int(ScreenRelais::ScreenType::CenteredScreen),
    int(ScreenRelais::ScreenType::DecenteredScreen),
    int(ScreenRelais::ScreenType::CenteredScreen),
    "ScreenRelais",
    {
        QT_TRANSLATE_NOOP("ScreenRelais", "Centered"),
        QT_TRANSLATE_NOOP("ScreenRelais", "Decentered")
    }
};

const EnumDesc &ScreenRelais::getTypeDesc()
{
    return screen_type_desc;
}

static const EnumDesc screen_glass_color_desc =
{
    int(ScreenRelais::GlassColor::Black),
    int(ScreenRelais::GlassColor::Green),
    int(ScreenRelais::GlassColor::Red),
    "ScreenRelais",
    {
        QT_TRANSLATE_NOOP("ScreenRelais", "Black"),
        QT_TRANSLATE_NOOP("ScreenRelais", "Red"),
        QT_TRANSLATE_NOOP("ScreenRelais", "Yellow"),
        QT_TRANSLATE_NOOP("ScreenRelais", "Green")
    }
};

const EnumDesc &ScreenRelais::getGlassColorDesc()
{
    return screen_glass_color_desc;
}

void ScreenRelais::setColorAt(int idx, GlassColor newColor)
{
    Q_ASSERT(idx >= 0 && idx <= 2);
    if(mColors[idx] == newColor)
        return;

    mColors[idx] = newColor;

    emit settingsChanged(this);
    emit typeChanged(this, mType);
}

void ScreenRelais::onReplicaModeChanged(bool on)
{
    if(!on)
    {
        // Return to local target position
        mTimer.start(std::chrono::milliseconds(50), this);
    }
}

ScreenRelais::ScreenRelais(AbstractSimulationObjectModel *m)
    : AbstractSimulationObject{m}
{

}

ScreenRelais::~ScreenRelais()
{
    if(mPowerNode)
    {
        mPowerNode->setScreenRelais(nullptr);
    }

    auto contactNodes = mContactNodes;
    for(ScreenRelaisContactNode *c : contactNodes)
    {
        c->setScreenRelais(nullptr);
    }

    mTimer.stop();
}

QString ScreenRelais::getType() const
{
    return Type;
}

bool ScreenRelais::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimulationObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Alredy created, nothing to do

    setScreenType(ScreenType(obj.value("screen_type").toInt(int(ScreenType::CenteredScreen))));

    setColorAt(0, GlassColor(obj.value("color_0").toInt(int(GlassColor::Yellow))));
    setColorAt(1, GlassColor(obj.value("color_1").toInt(int(GlassColor::Red))));
    setColorAt(2, GlassColor(obj.value("color_2").toInt(int(GlassColor::Green))));

    return true;
}

void ScreenRelais::saveToJSON(QJsonObject &obj) const
{
    AbstractSimulationObject::saveToJSON(obj);

    obj["screen_type"] = int(screenType());

    obj["color_0"] = int(getColorAt(0));
    obj["color_1"] = int(getColorAt(1));
    obj["color_2"] = int(getColorAt(2));
}

bool ScreenRelais::setReplicaState(const QCborMap &replicaState)
{
    setPosition(replicaState.value(QLatin1StringView("pos")).toDouble());
    return true;
}

void ScreenRelais::getReplicaState(QCborMap &replicaState) const
{
    replicaState[QLatin1StringView("pos")] = getPosition();
}

int ScreenRelais::getReferencingNodes(QVector<AbstractCircuitNode *> *result) const
{
    int nodesCount = AbstractSimulationObject::getReferencingNodes(result);

    nodesCount += mContactNodes.size();

    if(result)
    {
        for(auto item : mContactNodes)
            result->append(item);
    }

    if(mPowerNode)
    {
        nodesCount++;
        if(result)
            result->append(mPowerNode);
    }

    return nodesCount;
}

void ScreenRelais::timerEvent(QTimerEvent *e)
{
    if(e->id() == mTimer.id())
    {
        if(isRemoteReplica() || qFuzzyCompare(mTargetPosition, mPosition))
        {
            mTimer.stop();
        }
        else
        {
            const double centerPosition = mType == ScreenType::CenteredScreen ? 0 : 1;
            const bool goingToCenter = qFuzzyCompare(mTargetPosition, centerPosition);

            double newPosition = mPosition;
            if(mTargetPosition > newPosition)
            {
                newPosition += 0.08;
            }
            else
            {
                newPosition -= 0.08;
            }

            if(goingToCenter && qAbs(mPosition - mTargetPosition) < 0.2)
                newPosition = centerPosition; // TODO: oscillate a bit

            setPosition(newPosition);
        }
        return;
    }

    QObject::timerEvent(e);
}

void ScreenRelais::setPowerState(PowerState newState)
{
    if(!mPowerNode)
        newState = PowerState::None;

    if(mState == newState)
        return;

    mState = newState;

    mTargetPosition = getTargetPosition(screenType(), mState);

    if(!mTimer.isActive())
        mTimer.start(std::chrono::milliseconds(50), this);
}

void ScreenRelais::setPosition(double newPosition)
{
    if(mType == ScreenType::CenteredScreen)
        newPosition = qBound(-1.0, newPosition, 1.0);
    else
        newPosition = qBound(0.0, newPosition, 2.0);

    if(qFuzzyCompare(mPosition, newPosition))
        return;

    mPosition = newPosition;

    if(qFuzzyCompare(mTargetPosition, mPosition))
    {
        mTimer.stop();
    }

    // Update contact state
    const auto stateA = ScreenRelaisContactNode::ContactState(getContactStateA());
    const auto stateB = ScreenRelaisContactNode::ContactState(getContactStateB());
    for(ScreenRelaisContactNode *node : std::as_const(mContactNodes))
    {
        node->setState(node->isContactA() ? stateA : stateB);
    }

    emit stateChanged(this);
}

double ScreenRelais::getTargetPosition(ScreenType type, PowerState state)
{
    switch (state)
    {
    case PowerState::None:
        return 0;
    case PowerState::Direct:
        return 1;
    case PowerState::Reversed:
    {
        if(type == ScreenType::CenteredScreen)
            return -1;

        // For decentered screens we go over
        return 2;
    }
    default:
        break;
    }

    Q_UNREACHABLE();
    return 0;
}

ScreenRelais::ScreenType ScreenRelais::screenType() const
{
    return mType;
}

void ScreenRelais::setScreenType(ScreenType newType)
{
    if(mType == newType)
        return;

    mType = newType;
    emit settingsChanged(this);
    emit typeChanged(this, mType);

    // Adjust position
    setPosition(getTargetPosition(screenType(), mState));

    // Update contact state
    const auto stateA = ScreenRelaisContactNode::ContactState(getContactStateA());
    const auto stateB = ScreenRelaisContactNode::ContactState(getContactStateB());
    for(ScreenRelaisContactNode *node : std::as_const(mContactNodes))
    {
        node->setState(node->isContactA() ? stateA : stateB);
    }

    emit stateChanged(this);
}

ScreenRelais::ContactState ScreenRelais::getContactStateA() const
{
    // Contact A: coppia 1^ Cat
    // Centered screen positions:
    // 0 (red) -> straight
    // 1 (yellow) -> rev
    // -1 (green) -> rev

    // Deentered screen positions:
    // 0 (red) -> straight
    // 1 (yellow) -> rev
    // 2 (green) -> rev

    // It's the same regardless of screen type
    // Because position range changes
    if(mPosition > 0.8)
        return ContactState::Reversed;
    if(mPosition > 0.2)
        return ContactState::Middle;

    return ContactState::Straight;
}

ScreenRelais::ContactState ScreenRelais::getContactStateB() const
{
    // Contact B: coppia avviso
    // Centered screen positions:
    // 0 (red) -> straight
    // 1 (yellow) -> straight
    // -1 (green) -> rev

    // Deentered screen positions:
    // 0 (red) -> straight
    // 1 (yellow) -> straight
    // 2 (green) -> rev

    switch (screenType())
    {
    case ScreenType::CenteredScreen:
    {
        if(mPosition < -0.8)
            return ContactState::Reversed;
        if(mPosition < -0.2)
            return ContactState::Middle;

        return ContactState::Straight;
    }
    case ScreenType::DecenteredScreen:
    {
        if(mPosition > 1.8)
            return ContactState::Reversed;
        if(mPosition > 1.2)
            return ContactState::Middle;

        return ContactState::Straight;
    }
    default:
        break;
    }

    return ContactState::Middle;
}

void ScreenRelais::setPowerNode(ScreenRelaisPowerNode *node)
{
    if(mPowerNode == node)
        return;

    if(mPowerNode)
    {
        ScreenRelaisPowerNode *oldPowerNode = mPowerNode;
        mPowerNode = nullptr;
        oldPowerNode->setScreenRelais(nullptr);
    }

    mPowerNode = node;

    if(!mPowerNode)
    {
        setPowerState(PowerState::None);
    }

    emit nodesChanged(this);
    emit settingsChanged(this);
}

void ScreenRelais::addContactNode(ScreenRelaisContactNode *node)
{
    Q_ASSERT_X(!mContactNodes.contains(node),
               "addContactNode", "already added");

    mContactNodes.append(node);

    const ContactState s = node->isContactA() ? getContactStateA() : getContactStateB();
    node->setState(ScreenRelaisContactNode::ContactState(s));

    emit nodesChanged(this);
    emit settingsChanged(this);
}

void ScreenRelais::removeContactNode(ScreenRelaisContactNode *node)
{
    Q_ASSERT_X(mContactNodes.contains(node),
               "removeContactNode", "not registered");
    Q_ASSERT_X(node->screenRelais() == this,
               "removeContactNode", "relay does not match");

    mContactNodes.removeOne(node);

    emit nodesChanged(this);
}
