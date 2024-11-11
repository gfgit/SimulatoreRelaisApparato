/**
 * src/views/modemanager.h
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

#ifndef MODEMANAGER_H
#define MODEMANAGER_H

#include <QObject>

#include "../enums/filemodes.h"

class NodeEditFactory;
class CircuitListModel;

class RelaisModel;
class GenericLeverModel;

class QJsonObject;

class ModeManager : public QObject
{
    Q_OBJECT
public:
    explicit ModeManager(QObject *parent = nullptr);

    inline FileMode mode() const
    {
        return mMode;
    }

    void setMode(FileMode newMode);

    inline bool fileNeedsSaving() const
    {
        // Prevent showing modified file while loading
        if(mMode == FileMode::LoadingFile)
            return false;
        return mFileWasEdited;
    }

    void setFileEdited();
    void resetFileEdited();

    NodeEditFactory *circuitFactory() const;

    CircuitListModel *circuitList() const;

    bool loadFromJSON(const QJsonObject &obj);
    void saveToJSON(QJsonObject &obj) const;
    void clearAll();

    RelaisModel *relaisModel() const;

    GenericLeverModel *leversModel() const;

    EditingSubMode editingSubMode() const;
    void setEditingSubMode(EditingSubMode newEditingMode);

signals:
    void modeChanged(FileMode newMode, FileMode oldMode);
    void fileEdited(bool val);

    void editingSubModeChanged(EditingSubMode oldMode, EditingSubMode newMode);

private:
    FileMode mMode = FileMode::Editing;
    EditingSubMode mEditingMode = EditingSubMode::Default;

    NodeEditFactory *mCircuitFactory;
    CircuitListModel *mCircuitList;

    RelaisModel *mRelaisModel;
    GenericLeverModel *mLeversModel;

    bool mFileWasEdited = false;
};

#endif // MODEMANAGER_H
