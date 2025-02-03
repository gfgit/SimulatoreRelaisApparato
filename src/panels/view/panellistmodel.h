/**
 * src/panels/view/panellistmodel.h
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

#ifndef PANEL_LISTMODEL_H
#define PANEL_LISTMODEL_H

#include <QAbstractTableModel>

#include "../../enums/filemodes.h"

class PanelScene;
class AbstractPanelItem;

class ModeManager;

class QJsonObject;

class PanelListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        NameCol = 0,
        LongNameCol,
        NCols
    };

    explicit PanelListModel(ModeManager *mgr, QObject *parent = nullptr);
    ~PanelListModel();

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

    PanelScene *addPanelScene(const QString& name);
    bool removeSceneAtRow(int row);

    void clear();

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    inline const QVector<PanelScene *> getScenes() const
    {
        return mPanelScenes;
    }

    inline PanelScene *sceneAtRow(int row) const
    {
        return mPanelScenes.value(row, nullptr);
    }

    PanelScene *sceneByName(const QString &name) const;

    bool loadFromJSON(const QJsonObject &obj);
    void saveToJSON(QJsonObject &obj) const;

signals:
    void itemEditRequested(AbstractPanelItem *item);

private slots:
    void onSceneNameChanged(const QString& name, PanelScene *scene);

    void setMode(FileMode newMode, FileMode oldMode);

    void setEditingSubMode(EditingSubMode oldMode, EditingSubMode newMode);

private:
    friend class PanelScene;
    void onSceneEdited();

    void clearInternal();

private:
    QVector<PanelScene *> mPanelScenes;

    ModeManager *mModeMgr;

    bool mHasUnsavedChanges = false;
};

#endif // PANEL_LISTMODEL_H
