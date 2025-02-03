/**
 * src/panels/view/panellistmodel.cpp
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

#include "panellistmodel.h"

#include "../panelscene.h"

#include "../../views/modemanager.h"

#include <QJsonObject>
#include <QJsonArray>

PanelListModel::PanelListModel(ModeManager *mgr, QObject *parent)
    : QAbstractTableModel(parent)
    , mModeMgr(mgr)
{
    connect(mModeMgr, &ModeManager::modeChanged,
            this, &PanelListModel::setMode);
    connect(mModeMgr, &ModeManager::editingSubModeChanged,
            this, &PanelListModel::setEditingSubMode);
}

PanelListModel::~PanelListModel()
{
    clear();
}

QVariant PanelListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case NameCol:
            return tr("Name");
        case LongNameCol:
            return tr("Long Name");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int PanelListModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : mPanelScenes.size();
}

int PanelListModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant PanelListModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mPanelScenes.size())
        return QVariant();

    PanelScene *scene = mPanelScenes.at(idx.row());

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (idx.column())
        {
        case NameCol:
            return scene->panelName();
        case LongNameCol:
            return scene->panelLongName();
        default:
            break;
        }
    }

    return QVariant();
}

bool PanelListModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return false;

    if (!idx.isValid() || idx.row() >= mPanelScenes.size())
        return false;

    PanelScene *scene = mPanelScenes.at(idx.row());

    if(role == Qt::EditRole)
    {
        switch (idx.column())
        {
        case NameCol:
            return scene->setPanelName(value.toString());
        case LongNameCol:
            // Always valid
            scene->setPanelLongName(value.toString());
            return true;
        default:
            break;
        }
    }

    return false;
}

Qt::ItemFlags PanelListModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;

    if (!idx.isValid() || idx.row() >= mPanelScenes.size())
        return f;

    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEnabled);

    if(mModeMgr->mode() == FileMode::Editing)
        f.setFlag(Qt::ItemIsEditable);

    return f;
}

bool PanelListModel::isNameAvailable(const QString &name) const
{
    return std::none_of(mPanelScenes.cbegin(),
                        mPanelScenes.cend(),
                        [name](PanelScene *s) -> bool
    {
        return s->panelName() == name;
    });
}

void PanelListModel::resetHasUnsavedChanges()
{
    if(!mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = false;

    for(PanelScene *scene : std::as_const(mPanelScenes))
        scene->setHasUnsavedChanges(false);
}

PanelScene *PanelListModel::addPanelScene(const QString &name)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return nullptr;

    QString trimmedName = name.trimmed();

    if(!isNameAvailable(trimmedName))
        return nullptr;

    PanelScene *scene = new PanelScene(this);
    scene->setPanelName(trimmedName);

    int row = mPanelScenes.size();
    beginInsertRows(QModelIndex(), row, row);

    mPanelScenes.append(scene);

    endInsertRows();

    connect(scene, &PanelScene::nameChanged,
            this, &PanelListModel::onSceneNameChanged);
    connect(scene, &PanelScene::longNameChanged,
            this, &PanelListModel::onSceneNameChanged);

    onSceneEdited();

    return scene;
}

bool PanelListModel::removeSceneAtRow(int row)
{
    if(mModeMgr->mode() != FileMode::Editing)
        return false;

    Q_ASSERT(row >= 0);

    beginRemoveRows(QModelIndex(), row, row);

    PanelScene *scene = mPanelScenes.takeAt(row);
    delete scene;

    endRemoveRows();

    onSceneEdited();

    return true;
}

void PanelListModel::clear()
{
    beginResetModel();

    clearInternal();

    endResetModel();
}

PanelScene *PanelListModel::sceneByName(const QString &name) const
{
    for(PanelScene *s : mPanelScenes)
    {
        if(s->panelName() == name)
            return s;
    }

    return nullptr;
}

bool PanelListModel::loadFromJSON(const QJsonObject &obj)
{
    beginResetModel();

    clearInternal();

    const QJsonArray arr = obj.value("scenes").toArray();
    for(const QJsonValue& v : arr)
    {
        PanelScene *scene = new PanelScene(this);
        if(!scene->loadFromJSON(v.toObject(), modeMgr()->panelFactory()))
        {
            delete scene;
            continue;
        }

        connect(scene, &PanelScene::nameChanged,
                this, &PanelListModel::onSceneNameChanged);
        mPanelScenes.append(scene);
    }

    // Initially sort by name
    std::sort(mPanelScenes.begin(),
              mPanelScenes.end(),
              [](PanelScene *a, PanelScene *b) -> bool
    {
        return a->panelName() < b->panelName();
    });

    endResetModel();

    return true;
}

void PanelListModel::saveToJSON(QJsonObject &obj) const
{
    QJsonArray arr;

    for(PanelScene *scene : std::as_const(mPanelScenes))
    {
        QJsonObject sceneObj;
        scene->saveToJSON(sceneObj);
        arr.append(sceneObj);
    }

    obj["scenes"] = arr;
}

void PanelListModel::onSceneNameChanged(const QString &, PanelScene *scene)
{
    int row = mPanelScenes.indexOf(scene);
    Q_ASSERT(row >= 0);

    QModelIndex first = index(row, NameCol);
    QModelIndex last = index(row, LongNameCol);
    emit dataChanged(first, last);
}

void PanelListModel::setMode(FileMode newMode, FileMode oldMode)
{
    for(PanelScene *scene : std::as_const(mPanelScenes))
    {
        scene->setMode(newMode, oldMode);
    }
}

void PanelListModel::setEditingSubMode(EditingSubMode oldMode, EditingSubMode newMode)
{
    for(PanelScene *scene : std::as_const(mPanelScenes))
    {
        scene->onEditingSubModeChanged(oldMode, newMode);
    }
}

void PanelListModel::onSceneEdited()
{
    if(mHasUnsavedChanges)
        return;

    mHasUnsavedChanges = true;
    modeMgr()->setFileEdited();
}

void PanelListModel::clearInternal()
{
    qDeleteAll(mPanelScenes);
    mPanelScenes.clear();
}
