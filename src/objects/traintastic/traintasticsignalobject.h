/**
 * src/objects/traintastic/traintasticsignalobject.h
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2025 Filippo Gentile
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

#ifndef TRAINTASTIC_SIGNAL_OBJECT_H
#define TRAINTASTIC_SIGNAL_OBJECT_H

#include "../abstractsimulationobject.h"

class ScreenRelais;
class AbstractRelais;
class LightBulbObject;

class TraintasticSignalObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    static constexpr int InvalidChannel = -1;
    static constexpr int InvalidAddress = -1;

    enum ScreenRelays
    {
        FirstScreen = 0,
        SecondScreen,
        ThirdScreen,
        NScreenRelays
    };

    enum BlinkRelays
    {
        FirstLight = 0,
        SecondLight,
        ThirdLight,
        AdvanceSignalFakeOn,
        AdvanceSignalBlinker,
        NBlinkRelays
    };

    enum AuxLights
    {
        ArrowLight = 0,
        RappelLight60,
        RappelLight100,
        NAuxLights
    };

    struct DirectionEntry
    {
        LightBulbObject *light = nullptr;
        char letter = ' ';
    };

    explicit TraintasticSignalObject(AbstractSimulationObjectModel *m);
    ~TraintasticSignalObject();

    static constexpr QLatin1String Type = QLatin1String("traintastic_main_signal");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    inline int channel() const { return mChannel; }
    inline int address() const { return mAddress; }

    void setChannel(int newChannel);
    void setAddress(int newAddress);

    ScreenRelais *getScreenRelaisAt(int i) const
    {
        assert(i >= 0 && i < NScreenRelays);
        return mScreenRelais[i];
    }

    void setScreenRelaisAt(int i, ScreenRelais *s);

    AbstractRelais *getBlinkRelaisAt(int i) const
    {
        assert(i >= 0 && i < NBlinkRelays);
        return mBlinkRelais[i];
    }

    void setBlinkRelaisAt(int i, AbstractRelais *s);

    LightBulbObject *auxLight(AuxLights l) const;
    void setAuxLight(LightBulbObject *newArrowLight, AuxLights l);

    QVector<DirectionEntry> directionLights() const;
    void setDirectionLights(const QVector<DirectionEntry> &newDirectionLights);

public slots:
    void sendStatusMsg();

private slots:
    void onScreenPosChanged(AbstractSimulationObject *s);
    void onScreenDestroyed(QObject *obj);

    void onBlinRelaisStateChanged(AbstractSimulationObject *s);
    void onBlinRelaisDestroyed(QObject *obj);

    void onAuxLightDestroyed(QObject *obj);

private:
    void setScreenPos(int idx, int glassPos);
    void setBlinkRelayState(int idx, bool up);

private:
    int mChannel = 0;
    int mAddress = InvalidAddress;

    // Screens
    ScreenRelais *mScreenRelais[NScreenRelays] = {nullptr};
    int mCurScreenPos[NScreenRelays] = {};

    // Screens blinker and Advance Signal
    AbstractRelais *mBlinkRelais[NBlinkRelays] = {nullptr};
    bool mBlinkRelaisUp[NBlinkRelays] = {false, false, false};

    // Square arrow
    LightBulbObject *mArrowLight = nullptr;

    // Rappel
    LightBulbObject *mRappelLight60 = nullptr;
    LightBulbObject *mRappelLight100 = nullptr;

    QVector<DirectionEntry> mDirectionLights;
};

inline bool operator==(const TraintasticSignalObject::DirectionEntry& lhs,
                       const TraintasticSignalObject::DirectionEntry& rhs)
{
    return lhs.light == rhs.light && lhs.letter == rhs.letter;
}

#endif // TRAINTASTIC_SIGNAL_OBJECT_H
