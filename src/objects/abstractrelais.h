/**
 * src/objects/abstractrelais.h
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

#ifndef ABSTRACTRELAIS_H
#define ABSTRACTRELAIS_H

#include <QObject>

class RelaisPowerNode;
class RelaisContactNode;

class QJsonObject;

class AbstractRelais : public QObject
{
    Q_OBJECT
public:

    enum class State
    {
        Up = 0,
        Down = 1,
        GoingUp = 2,
        GoingDown = 3
    };

    explicit AbstractRelais(QObject *parent = nullptr);
    ~AbstractRelais();

    virtual bool loadFromJSON(const QJsonObject& obj);
    virtual void saveToJSON(QJsonObject& obj) const;

    State state() const;
    void setState(State newState);

    QString name() const;

    void setName(const QString &newName);

    double upSpeed() const;
    void setUpSpeed(double newUpSpeed);

    double downSpeed() const;
    void setDownSpeed(double newDownSpeed);

    void timerEvent(QTimerEvent *e) override;

signals:
    void nameChanged(AbstractRelais *self, const QString& name);
    void stateChanged(AbstractRelais *self, State s);

private:
    friend class RelaisPowerNode;
    void addPowerNode(RelaisPowerNode *p);
    void removePowerNode(RelaisPowerNode *p);

    friend class RelaisContactNode;
    void addContactNode(RelaisContactNode *c);
    void removeContactNode(RelaisContactNode *c);

    void powerNodeActivated(RelaisPowerNode *p);
    void powerNodeDeactivated(RelaisPowerNode *p);

    void setPosition(double newPosition);
    void startMove(bool up);

private:
    QString mName;
    State mState = State::Down;
    State mInternalState = State::Down;

    // Steps per 250ms (Speed of 0.25 means it goes up in 1 sec)
    double mUpSpeed = 0.18;
    double mDownSpeed = 0.25;
    double mPosition = 0.0;
    int mTimerId = 0;

    QVector<RelaisPowerNode *> mPowerNodes;
    int mActivePowerNodes = 0;

    QVector<RelaisContactNode *> mContactNodes;
};

#endif // ABSTRACTRELAIS_H
