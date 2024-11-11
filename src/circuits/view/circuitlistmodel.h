/**
 * src/circuits/view/circuitlistmodel.h
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

#ifndef CIRCUITLISTMODEL_H
#define CIRCUITLISTMODEL_H

#include <QAbstractTableModel>

#include "../../enums/filemodes.h"

class CircuitScene;
class AbstractNodeGraphItem;
class CableGraphItem;

class ModeManager;

class QJsonObject;

class CircuitListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        NameCol = 0,
        LongNameCol,
        NCols
    };

    explicit CircuitListModel(ModeManager *mgr, QObject *parent = nullptr);
    ~CircuitListModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    bool isNameAvailable(const QString& name) const;

    inline bool hasUnsavedChanges() const
    {
        return mHasUnsavedChanges;
    }

    void resetHasUnsavedChanges();

    CircuitScene *addCircuitScene(const QString& name);
    bool removeSceneAtRow(int row);

    void clear();

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    inline const QVector<CircuitScene *> getScenes() const
    {
        return mCircuitScenes;
    }

    inline CircuitScene *sceneAtRow(int row) const
    {
        return mCircuitScenes.value(row, nullptr);
    }

    bool loadFromJSON(const QJsonObject &obj);
    void saveToJSON(QJsonObject &obj) const;

signals:
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

private slots:
    void onSceneNameChanged(const QString& name, CircuitScene *scene);

    void setMode(FileMode newMode, FileMode oldMode);

    void setEditingSubMode(EditingSubMode oldMode, EditingSubMode newMode);

private:
    friend class CircuitScene;
    void onSceneEdited();

    void clearInternal();

private:
    QVector<CircuitScene *> mCircuitScenes;

    ModeManager *mModeMgr;

    bool mHasUnsavedChanges = false;
};

#endif // CIRCUITLISTMODEL_H
