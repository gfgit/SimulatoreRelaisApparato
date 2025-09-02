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

class TraintasticSignalObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    static constexpr int InvalidChannel = -1;
    static constexpr int InvalidAddress = -1;

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
        assert(i >= 0 && i < 3);
        return mScreenRelais[i];
    }

    void setScreenRelaisAt(int i, ScreenRelais *s);

    void sendStatusMsg();

private slots:
    void onScreenPosChanged(AbstractSimulationObject *s);
    void onScreenDestroyed(QObject *obj);

private:
    void setScreenPos(int idx, int glassPos);

private:
    int mChannel = 0;
    int mAddress = InvalidAddress;

    ScreenRelais *mScreenRelais[3] = {nullptr};
    int mCurScreenPos[3] = {};
};

#endif // TRAINTASTIC_SIGNAL_OBJECT_H
