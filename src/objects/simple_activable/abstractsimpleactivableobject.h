/**
 * src/objects/simple_activable/abstractsimpleactivableobject.h
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

#ifndef ABSTRACTSIMPLEACTIVABLEOBJECT_H
#define ABSTRACTSIMPLEACTIVABLEOBJECT_H

#include "../abstractsimulationobject.h"

#include <QVector>

class SimpleActivationNode;

class AbstractSimpleActivableObject : public AbstractSimulationObject
{
    Q_OBJECT
public:
    enum class State
    {
        Off = 0,
        On = 1
    };


    explicit AbstractSimpleActivableObject(AbstractSimulationObjectModel *m);
    ~AbstractSimpleActivableObject();

    bool event(QEvent *e) override;

    int getReferencingNodes(QVector<AbstractCircuitNode *> *result) const override;

    virtual State state() const;

    void applyDelayedStateChanged();

private:
    friend class SimpleActivationNode;
    void addNode(SimpleActivationNode *node);
    void removeNode(SimpleActivationNode *node);

    void onNodeStateChanged(SimpleActivationNode *node, bool val);

protected:
    virtual void onStateChangedInternal();

private:
    QVector<SimpleActivationNode *> mNodes;

    // Allow multiple powering nodes
    // TODO: is this useful?
    int mActiveNodesCount = 0;
    int mPendingNodesCount = 0;
    bool mUpdateScheduled = false;
};

#endif // ABSTRACTSIMPLEACTIVABLEOBJECT_H
