/**
 * src/objects/simulationobjectmultitypemodel.cpp
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

#include "simulationobjectmultitypemodel.h"

#include "abstractsimulationobjectmodel.h"
#include "simulationobjectfactory.h"

#include "../views/modemanager.h"

SimulationObjectMultiTypeModel::SimulationObjectMultiTypeModel(ModeManager *mgr, const QStringList& types,
                                                               QObject *parent)
    : QAbstractTableModel(parent)
    , modeMgr(mgr)
{
    for(const QString& typeName : types)
    {
        auto m = modeMgr->modelForType(typeName);
        if(!m)
            continue;

        mModels.append(m);

        connect(m, &AbstractSimulationObjectModel::rowsAboutToBeInserted,
                this, &SimulationObjectMultiTypeModel::onBeginReset);
        connect(m, &AbstractSimulationObjectModel::rowsAboutToBeMoved,
                this, &SimulationObjectMultiTypeModel::onBeginReset);
        connect(m, &AbstractSimulationObjectModel::rowsAboutToBeRemoved,
                this, &SimulationObjectMultiTypeModel::onBeginReset);
        connect(m, &AbstractSimulationObjectModel::modelAboutToBeReset,
                this, &SimulationObjectMultiTypeModel::onBeginReset);

        connect(m, &AbstractSimulationObjectModel::rowsInserted,
                this, &SimulationObjectMultiTypeModel::onResetEnd);
        connect(m, &AbstractSimulationObjectModel::rowsMoved,
                this, &SimulationObjectMultiTypeModel::onResetEnd);
        connect(m, &AbstractSimulationObjectModel::rowsRemoved,
                this, &SimulationObjectMultiTypeModel::onResetEnd);
        connect(m, &AbstractSimulationObjectModel::modelReset,
                this, &SimulationObjectMultiTypeModel::onResetEnd);
    }

    // Trigger refresh
    onBeginReset();
    onResetEnd();
}

QVariant SimulationObjectMultiTypeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return tr("Name");
        case 1:
            return tr("Type");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int SimulationObjectMultiTypeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mCachedRowCount;
}

int SimulationObjectMultiTypeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 2;
}

QVariant SimulationObjectMultiTypeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int sourceRow = 0;
    const AbstractSimulationObjectModel *m = getSourceModel(index.row(), sourceRow);
    if(!m)
        return QVariant();

    if(index.column() == 0)
        return m->data(m->index(sourceRow, 0), role);
    else if((index.column() == 1 && role == Qt::DisplayRole) || role == Qt::ToolTipRole)
        return modeMgr->objectFactory()->prettyName(m->getObjectType());

    return QVariant();
}

AbstractSimulationObject *SimulationObjectMultiTypeModel::objectAt(int row) const
{
    int sourceRow = 0;
    const AbstractSimulationObjectModel *m = getSourceModel(row, sourceRow);
    if(!m)
        return nullptr;

    return m->objectAt(sourceRow);
}

AbstractSimulationObject *SimulationObjectMultiTypeModel::getObjectByName(const QString &name) const
{
    for(AbstractSimulationObjectModel *m : std::as_const(mModels))
    {
        if(auto *obj = m->getObjectByName(name))
            return obj;
    }
    return nullptr;
}

void SimulationObjectMultiTypeModel::onBeginReset()
{
    mResetCount++;
    if(mResetCount == 1)
        beginResetModel();
}

void SimulationObjectMultiTypeModel::onResetEnd()
{
    mResetCount--;
    if(mResetCount == 0)
    {
        int total = 0;
        for(const AbstractSimulationObjectModel *m : std::as_const(mModels))
            total += m->rowCount();
        mCachedRowCount = total;

        endResetModel();
    }
}

AbstractSimulationObjectModel *SimulationObjectMultiTypeModel::getSourceModel(int row, int &outSourceRow) const
{
    int modelFirstRow = 0;
    for(AbstractSimulationObjectModel *m : std::as_const(mModels))
    {
        int count = m->rowCount();
        if(count <= 0)
            continue;

        int lastRow = modelFirstRow + count - 1;

        if(row <= lastRow)
        {
            outSourceRow = row - modelFirstRow;
            return m;
        }

        modelFirstRow = lastRow + 1;
    }

    return nullptr;
}
