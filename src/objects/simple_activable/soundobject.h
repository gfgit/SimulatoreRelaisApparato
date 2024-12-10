/**
 * src/objects/simple_activable/soundobject.h
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

#ifndef SOUNDOBJECT_H
#define SOUNDOBJECT_H

#include "abstractsimpleactivableobject.h"

#include <QVector>

class QSoundEffect;

class QPropertyAnimation;

class SoundObject : public AbstractSimpleActivableObject
{
public:
    enum SoundState
    {
        Stopped = 0,
        Playing = 1,
        FadeIn = 2,
        FadeOut = 3
    };

    explicit SoundObject(AbstractSimulationObjectModel *m);
    ~SoundObject();

    static constexpr QLatin1String Type = QLatin1String("sound_activable");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject& obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject& obj) const override;

    SoundState soundState() const;
    void setSoundState(SoundState newState);

    QString getSoundFile() const;
    void setSoundFile(const QString &fileName);

    bool loopEnabled() const;
    void setLoopEnabled(bool newLoopEnabled);

private slots:
    void onFadeEnd();
    void onPlayingChanged();

protected:
    virtual void onStateChangedInternal();

private:
    QSoundEffect *mSound = nullptr;
    QPropertyAnimation *mFadeAnimation = nullptr;
    QVector<QSoundEffect *> mOverlappedSounds;

    SoundState mSoundState = SoundState::Stopped;
    bool stoppedByNewState = false;

    bool mLoopEnabled = true;
};

#endif // SOUNDOBJECT_H
