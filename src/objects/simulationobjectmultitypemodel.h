/**
 * src/objects/simulationobjectmultitypemodel.h
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

#ifndef SIMULATIONOBJECTMULTITYPEMODEL_H
#define SIMULATIONOBJECTMULTITYPEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class AbstractSimulationObject;
class AbstractSimulationObjectModel;
class ModeManager;

class SimulationObjectMultiTypeModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SimulationObjectMultiTypeModel(ModeManager *mgr, const QStringList& types,
                                   QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    AbstractSimulationObject *objectAt(int row) const;

    AbstractSimulationObject *getObjectByName(const QString& name) const;

private slots:
    void onBeginReset();
    void onResetEnd();

private:
    AbstractSimulationObjectModel* getSourceModel(int row, int& outSourceRow) const;

private:
    ModeManager *modeMgr;
    QVector<AbstractSimulationObjectModel *> mModels;

    int mResetCount = 0;
    int mCachedRowCount = 0;
};

#endif // SIMULATIONOBJECTMULTITYPEMODEL_H
