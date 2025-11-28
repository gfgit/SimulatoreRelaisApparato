/**
 * src/objects/traintastic/traintasticauxsignalobject.h
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

#ifndef TRAINTASTIC_AUX_SIGNAL_OBJECT_H
#define TRAINTASTIC_AUX_SIGNAL_OBJECT_H

#include "../abstractsimulationobject.h"

class LightBulbObject;

class TraintasticAuxSignalObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    static constexpr int InvalidChannel = -1;
    static constexpr int InvalidAddress = -1;

    enum class AuxLights
    {
        L1 = 0,
        L2,
        L3,
        NAuxLights
    };

    explicit TraintasticAuxSignalObject(AbstractSimulationObjectModel *m);
    ~TraintasticAuxSignalObject();

    static constexpr QLatin1String Type = QLatin1String("traintastic_aux_signal");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    inline int channel() const { return mChannel; }
    inline int address() const { return mAddress; }

    void setChannel(int newChannel);
    void setAddress(int newAddress);

    LightBulbObject *auxLight(AuxLights l) const;
    void setAuxLight(LightBulbObject *newArrowLight, AuxLights l);

public slots:
    void sendStatusMsg();

private slots:
    void onAuxLightDestroyed(QObject *obj);

private:
    int mChannel = 0;
    int mAddress = InvalidAddress;

    // Lights
    LightBulbObject *mLights[int(AuxLights::NAuxLights)] = {nullptr};
};

#endif // TRAINTASTIC_AUX_SIGNAL_OBJECT_H
