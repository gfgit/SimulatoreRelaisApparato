/**
 * src/objects/traintastic/traintasticspawnobj.h
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

#ifndef TRAINTASTICSPAWNOBJ_H
#define TRAINTASTICSPAWNOBJ_H

#include "../abstractsimulationobject.h"

#include <QBasicTimer>

class TraintasticTurnoutNode;

class TraintasticSpawnObj : public AbstractSimulationObject
{
    Q_OBJECT
public:
    static constexpr int InvalidAddress = -1;

    explicit TraintasticSpawnObj(AbstractSimulationObjectModel *m);
    ~TraintasticSpawnObj();

    static constexpr QLatin1String Type = QLatin1String("traintastic_spawn");
    QString getType() const override;

    bool loadFromJSON(const QJsonObject &obj, LoadPhase phase) override;
    void saveToJSON(QJsonObject &obj) const override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    const TraintasticTurnoutNode *getNode() const { return mNode; }

    inline int address() const { return mAddress; }
    bool setAddress(int newAddress);

    inline bool isActive() const { return mActive; }

private:
    friend class TraintasticTurnoutNode;
    void setNode(TraintasticTurnoutNode *node);
    void setActive(bool val);

private:
    TraintasticTurnoutNode *mNode = nullptr;

    int mAddress = InvalidAddress;
    bool mActive = false;
};

#endif // TRAINTASTICSPAWNOBJ_H
