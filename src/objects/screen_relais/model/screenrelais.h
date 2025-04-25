/**
 * src/objects/screen_relais/model/screenrelais.h
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

#ifndef SCREEN_RELAIS_H
#define SCREEN_RELAIS_H

#include "../../abstractsimulationobject.h"

#include <QBasicTimer>

class ScreenRelaisPowerNode;
class ScreenRelaisContactNode;

class QJsonObject;

class EnumDesc;

class ScreenRelais : public AbstractSimulationObject
{
    Q_OBJECT
public:
    static constexpr int DefaultUpMS = 700;
    static constexpr int DefaultDownMS = 200;

    enum class ContactState
    {
        Straight = 0,
        Middle = 1,
        Reversed = 2
    };

    enum class PowerState
    {
        None = 0,
        Direct = 1,
        Reversed = 2
    };

    enum class ScreenType
    {
        CenteredScreen = 0,
        DecenteredScreen,
        NTypes
    };

    enum class GlassColor
    {
        Black  = 0,
        Red    = 1,
        Yellow = 2,
        Green  = 3,
        NColors
    };

    static QString getScreenTypeName(ScreenType t);

    explicit ScreenRelais(AbstractSimulationObjectModel *m);
    ~ScreenRelais();

    static constexpr QLatin1String Type = QLatin1String("screen_relais");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject& obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject& obj) const override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    void timerEvent(QTimerEvent *e) override;

    ScreenType screenType() const;
    void setScreenType(ScreenType newType);

    inline double getPosition() const { return mPosition; }

    ContactState getContactStateA() const;
    ContactState getContactStateB() const;

    static const EnumDesc &getTypeDesc();
    static const EnumDesc &getGlassColorDesc();

    inline GlassColor getColorAt(int idx) const
    {
        Q_ASSERT(idx >= 0 && idx <= 2);
        return mColors[idx];
    }

    void setColorAt(int idx, GlassColor newColor);

    inline int getContactNodesCount() const
    {
        return mContactNodes.size();
    }

    inline bool hasPowerNode() const
    {
        return mPowerNode;
    }

signals:
    void typeChanged(ScreenRelais *self, ScreenType s);

private:
    friend class ScreenRelaisPowerNode;
    friend class ScreenRelaisContactNode;

    void setPowerNode(ScreenRelaisPowerNode *node);

    void addContactNode(ScreenRelaisContactNode *node);
    void removeContactNode(ScreenRelaisContactNode *node);

    void setPowerState(PowerState newState);
    void setPosition(double newPosition);

    static double getTargetPosition(ScreenType type, PowerState state);

private:
    ScreenType mType = ScreenType::CenteredScreen;
    PowerState mState = PowerState::None;

    double mTickPositionDelta = 0;

    double mPosition = 0.0;
    double mTargetPosition = 0.0;

    QBasicTimer mTimer;

    ScreenRelaisPowerNode *mPowerNode = nullptr;

    QVector<ScreenRelaisContactNode *> mContactNodes;

    GlassColor mColors[3] = {GlassColor::Yellow,
                             GlassColor::Red,
                             GlassColor::Green};
};

#endif // SCREEN_RELAIS_H
