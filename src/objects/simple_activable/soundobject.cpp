/**
 * src/objects/simple_activable/soundobject.cpp
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

#include "soundobject.h"

#include "../abstractsimulationobjectmodel.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

#include <QDir>

#include <QSoundEffect>

#include <QPropertyAnimation>


SoundObject::SoundObject(AbstractSimulationObjectModel *m)
    : AbstractSimpleActivableObject(m)
{
    mSound = new QSoundEffect(this);
    mSound->setVolume(0);

    mFadeAnimation = new QPropertyAnimation(mSound, "volume", this);
    connect(mFadeAnimation, &QPropertyAnimation::finished,
            this, &SoundObject::onFadeEnd);
    connect(mSound, &QSoundEffect::playingChanged,
            this, &SoundObject::onPlayingChanged);

    mSound->setLoopCount(QSoundEffect::Infinite);
}

SoundObject::~SoundObject()
{
    const auto overlappedCopy = mOverlappedSounds;
    for(QSoundEffect *sound : overlappedCopy)
    {
        sound->stop();
        delete sound;
    };
    Q_ASSERT(mOverlappedSounds.isEmpty());

    mSound->stop();
    delete mSound;
    mSound = nullptr;
}

QString SoundObject::getType() const
{
    return Type;
}

bool SoundObject::loadFromJSON(const QJsonObject &obj, LoadPhase phase)
{
    if(!AbstractSimpleActivableObject::loadFromJSON(obj, phase))
        return false;

    if(phase != LoadPhase::Creation)
        return true; // Nothing to do

    setLoopEnabled(obj.value("loop").toBool(true));

    QString soundPath = obj.value("sound_file").toString();
    if(!soundPath.isEmpty())
    {
        QFileInfo soundFileInfo(soundPath);
        if(soundFileInfo.isRelative())
        {
            const QString jsonPath = model()->modeMgr()->filePath();
            QFileInfo info(jsonPath);
            soundPath = info.absoluteDir().absoluteFilePath(soundPath);
        }
    }

    setSoundFile(soundPath);

    return true;
}

void SoundObject::saveToJSON(QJsonObject &obj) const
{
    AbstractSimpleActivableObject::saveToJSON(obj);

    obj["loop"] = loopEnabled();

    QString soundPath = getSoundFile();

    const QString jsonPath = model()->modeMgr()->filePath();
    if(!jsonPath.isEmpty())
    {
        QFileInfo info(jsonPath);
        if(info.exists())
        {
            soundPath = info.absoluteDir().relativeFilePath(soundPath);
        }
    }

    obj["sound_file"] = soundPath;
}

SoundObject::SoundState SoundObject::soundState() const
{
    return mSoundState;
}

void SoundObject::setSoundState(SoundState newState)
{
    if(newState == mSoundState)
        return;

    if(mLoopEnabled)
    {
        // For loops we implement fade in/fade out
        // on node activation/deactivation
        if(newState == SoundState::Playing && mSoundState != SoundState::FadeIn)
        {
            setSoundState(SoundState::FadeIn);
            return;
        }

        if(newState == SoundState::Stopped && mSoundState != SoundState::FadeOut)
        {
            setSoundState(SoundState::FadeOut);
            return;
        }
    }

    // Stop animation
    stoppedByNewState = true;
    mFadeAnimation->stop();
    stoppedByNewState = false;

    mSoundState = newState;

    if(mSoundState == SoundState::Stopped)
    {
        // Let single sound go on until end
        // Stop only loop sounds
        if(mLoopEnabled)
            mSound->stop();
    }
    else if(mSoundState == SoundState::Playing)
    {
        if(mLoopEnabled)
        {
            // Nothing to do, FadeIn already put as at max volume
        }
        else
        {
            QSoundEffect *soundEffect = mSound;
            if(mSound->isPlaying() && mOverlappedSounds.size() < 3)
            {
                // FIXME: still not working properly
                // Trigger ALSA error if played rapidly many times
                // "ALSA lib pcm.c:8675:(snd_pcm_recover) underrun occurred"

                // Our sound is already playing
                // Probably because node has stopped
                // but audio sample is not finished yet

                // We create a new sound of same file and play it over
                // original

                soundEffect = new QSoundEffect(this);
                soundEffect->setSource(mSound->source());
                mOverlappedSounds.append(soundEffect);
                connect(soundEffect, &QSoundEffect::playingChanged,
                        [soundEffect]()
                {
                    if(!soundEffect->isPlaying())
                        soundEffect->deleteLater();
                });
                connect(soundEffect, &QObject::destroyed,
                        [this, soundEffect]()
                {
                    mOverlappedSounds.removeOne(soundEffect);
                });
            }

            soundEffect->setVolume(1.0);
            soundEffect->play();
        }
    }
    else if(mSoundState == SoundState::FadeIn)
    {
        if(!mSound->isPlaying())
            mFadeAnimation->setStartValue(0.2);
        else
            mFadeAnimation->setStartValue(mSound->volume());
        mFadeAnimation->setEndValue(1.0);
        mFadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
        mFadeAnimation->setDuration(500);
        mFadeAnimation->start();

        // Start playing
        if(!mSound->isPlaying())
            mSound->play();
    }
    else if(mSoundState == SoundState::FadeOut)
    {
        // Do not stop playing yet
        mFadeAnimation->setStartValue(mSound->volume());
        mFadeAnimation->setEndValue(0.0);
        mFadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
        mFadeAnimation->setDuration(1000);
        mFadeAnimation->start();
    }

    emit stateChanged(this);
}

QString SoundObject::getSoundFile() const
{
    return mSound->source().toLocalFile();
}

void SoundObject::setSoundFile(const QString &fileName)
{
    QFileInfo info(fileName);
    mSound->setSource(QUrl::fromLocalFile(info.canonicalFilePath()));
}

bool SoundObject::loopEnabled() const
{
    return mLoopEnabled;
}

void SoundObject::setLoopEnabled(bool newLoopEnabled)
{
    if(mLoopEnabled == newLoopEnabled)
        return;

    mLoopEnabled = newLoopEnabled;

    mSound->setLoopCount(mLoopEnabled ? QSoundEffect::Infinite : 1);

    emit settingsChanged(this);
}

void SoundObject::onFadeEnd()
{
    if(stoppedByNewState)
        return;

    if(mSoundState == SoundState::FadeIn)
        setSoundState(SoundState::Playing);
    else if(mSoundState == SoundState::FadeOut)
        setSoundState(SoundState::Stopped);
}

void SoundObject::onPlayingChanged()
{
    if(!mSound->isPlaying() && !stoppedByNewState)
        setSoundState(SoundState::Stopped);
}

void SoundObject::onStateChangedInternal()
{
    if(state() == State::On)
        setSoundState(SoundState::Playing);
    else
        setSoundState(SoundState::Stopped);
}
