/**
 * src/views/modemanager.cpp
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

#include "modemanager.h"

#include "../circuits/edit/nodeeditfactory.h"
#include "../circuits/edit/standardnodetypes.h"

#include "../circuits/view/circuitlistmodel.h"

#include "../objects/relais/model/relaismodel.h"

#include <QJsonObject>

ModeManager::ModeManager(QObject *parent)
    : QObject{parent}
{
    mRelaisModel = new RelaisModel(this, this);

    mCircuitFactory = new NodeEditFactory(this);
    StandardNodeTypes::registerTypes(mCircuitFactory);

    mCircuitList = new CircuitListModel(this, this);
}

void ModeManager::setMode(FileMode newMode)
{
    if (mMode == newMode)
        return;

    FileMode oldMode = mMode;
    mMode = newMode;
    emit modeChanged(mMode, oldMode);

    // Update file edited because it depends on mode
    emit fileEdited(fileNeedsSaving());
}

void ModeManager::setFileEdited()
{
    if(mFileWasEdited)
        return;

    mFileWasEdited = true;

    if(mode() != FileMode::LoadingFile)
        emit fileEdited(true);
}

void ModeManager::resetFileEdited()
{
    if(!mFileWasEdited)
        return;

    mFileWasEdited = false;

    mCircuitList->resetHasUnsavedChanges();
    mRelaisModel->resetHasUnsavedChanges();

    emit fileEdited(false);
}

NodeEditFactory *ModeManager::circuitFactory() const
{
    return mCircuitFactory;
}

CircuitListModel *ModeManager::circuitList() const
{
    return mCircuitList;
}

bool ModeManager::loadFromJSON(const QJsonObject &obj)
{
    // Temporarily ignore modified scenes
    setMode(FileMode::LoadingFile);

    QJsonObject relais = obj.value("relais").toObject();
    mRelaisModel->loadFromJSON(relais);

    QJsonObject circuits = obj.value("circuits").toObject();
    mCircuitList->loadFromJSON(circuits);

    resetFileEdited();

    // Turn on power sources and stuff
    setMode(FileMode::Simulation);

    return true;
}

void ModeManager::saveToJSON(QJsonObject &obj) const
{
    QJsonObject circuits;
    mCircuitList->saveToJSON(circuits);

    QJsonObject relais;
    mRelaisModel->saveToJSON(relais);

    obj["circuits"] = circuits;
    obj["relais"] = relais;
}

void ModeManager::clearAll()
{
    setMode(FileMode::LoadingFile);

    mCircuitList->clear();
    mRelaisModel->clear();

    resetFileEdited();

    // Wait for new items
    setMode(FileMode::Editing);
}

RelaisModel *ModeManager::relaisModel() const
{
    return mRelaisModel;
}
