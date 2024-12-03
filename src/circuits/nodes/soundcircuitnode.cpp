/**
 * src/circuits/nodes/soundcircuitnode.cpp
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

#include "soundcircuitnode.h"

#include "../../views/modemanager.h"

#include <QJsonObject>

#include <QDir>

#include <QSoundEffect>

#include <QPropertyAnimation>

SoundCircuitNode::SoundCircuitNode(ModeManager *mgr, QObject *parent)
    : AbstractCircuitNode{mgr, true, parent}
{
    // 1 side
    mContacts.append(NodeContact("1", "2"));

    mSound = new QSoundEffect(this);
    mSound->setVolume(0);

    mFadeAnimation = new QPropertyAnimation(mSound, "volume", this);
    connect(mFadeAnimation, &QPropertyAnimation::finished,
            this, &SoundCircuitNode::onFadeEnd);
}

QVector<CableItem> SoundCircuitNode::getActiveConnections(CableItem source, bool invertDir)
{
    if(source.nodeContact != 0 || !mContacts.at(0).cable)
        return {};

    // Close the circuit
    CableItem dest;
    dest.cable.cable = mContacts.at(0).cable;
    dest.cable.side = mContacts.at(0).cableSide;
    dest.nodeContact = 0;
    dest.cable.pole = ~source.cable.pole; // Invert pole
    return {dest};
}

void SoundCircuitNode::addCircuit(ElectricCircuit *circuit)
{
    const bool wasActive = hasCircuits();

    AbstractCircuitNode::addCircuit(circuit);

    const bool isActive = hasCircuits();

    if(!wasActive && isActive)
    {
        setState(State::Playing);
    }
}

void SoundCircuitNode::removeCircuit(ElectricCircuit *circuit, const NodeOccurences &items)
{
    const bool wasActive = hasCircuits();

    AbstractCircuitNode::removeCircuit(circuit, items);

    const bool isActive = hasCircuits();

    if(wasActive && !isActive)
    {
        setState(State::Stopped);
    }
}

bool SoundCircuitNode::loadFromJSON(const QJsonObject &obj)
{
    if(!AbstractCircuitNode::loadFromJSON(obj))
        return false;

    QString soundPath = obj.value("sound_file").toString();
    if(!soundPath.isEmpty())
    {
        QFileInfo soundFileInfo(soundPath);
        if(soundFileInfo.isRelative())
        {
            const QString jsonPath = modeMgr()->filePath();
            QFileInfo info(jsonPath);
            soundPath = info.absoluteDir().absoluteFilePath(soundPath);
        }
    }

    setSoundFile(soundPath);

    return true;
}

void SoundCircuitNode::saveToJSON(QJsonObject &obj) const
{
    AbstractCircuitNode::saveToJSON(obj);


    QString soundPath = getSoundFile();

    const QString jsonPath = modeMgr()->filePath();
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

QString SoundCircuitNode::nodeType() const
{
    return NodeType;
}

QString SoundCircuitNode::getSoundFile() const
{
    return mSound->source().toLocalFile();
}

void SoundCircuitNode::setSoundFile(const QString& fileName)
{
    QFileInfo info(fileName);
    mSound->setSource(QUrl::fromLocalFile(info.canonicalFilePath()));
}

void SoundCircuitNode::onFadeEnd()
{
    if(stoppedByNewState)
        return;

    if(mState == State::FadeIn)
        setState(State::Playing);
    else if(mState == State::FadeOut)
        setState(State::Stopped);
}

void SoundCircuitNode::setState(State newState)
{
    if(newState == mState)
        return;

    if(newState == State::Playing && mState != State::FadeIn)
    {
        setState(State::FadeIn);
        return;
    }

    if(newState == State::Stopped && mState != State::FadeOut)
    {
        setState(State::FadeOut);
        return;
    }

    stoppedByNewState = true;
    mFadeAnimation->stop();
    stoppedByNewState = false;

    mState = newState;

    if(mState == State::Stopped)
    {
        mSound->stop();
    }
    else if(mState == State::Playing)
    {
        // Nothing to do
    }
    else if(mState == State::FadeIn)
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
        mSound->setLoopCount(QSoundEffect::Infinite);
        if(!mSound->isPlaying())
            mSound->play();
    }
    else if(mState == State::FadeOut)
    {
        // Do not stop playing yet
        mFadeAnimation->setStartValue(mSound->volume());
        mFadeAnimation->setEndValue(0.0);
        mFadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
        mFadeAnimation->setDuration(1000);
        mFadeAnimation->start();
    }
}
