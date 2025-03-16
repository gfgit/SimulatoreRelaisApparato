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

#include <QHash>

#include "../enums/filemodes.h"

class NodeEditFactory;
class CircuitListModel;

class PanelItemFactory;
class PanelListModel;

class SimulationObjectFactory;
class AbstractSimulationObjectModel;

class QJsonObject;

class RemoteManager;
class SerialManager;

class ModeManager : public QObject
{
    Q_OBJECT
public:
    enum FileVersion
    {
        Beta = 0,
        V1 = 1,
        Current = V1
    };

    explicit ModeManager(QObject *parent = nullptr);
    ~ModeManager();

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

    PanelItemFactory *panelFactory() const;
    PanelListModel *panelList() const;

    bool loadFromJSON(const QJsonObject &obj);
    void saveToJSON(QJsonObject &obj) const;
    void clearAll();

    EditingSubMode editingSubMode() const;
    void setEditingSubMode(EditingSubMode newEditingMode);

    SimulationObjectFactory *objectFactory() const;

    AbstractSimulationObjectModel *modelForType(const QString& objType) const;

    QString filePath() const;
    void setFilePath(const QString &newFilePath, bool newFile = false);

    inline RemoteManager *getRemoteManager() const
    {
        return mRemoteMgr;
    }

    inline SerialManager *getSerialManager() const
    {
        return mSerialMgr;
    }

signals:
    void fileChanged(const QString& newFile, const QString& oldFile);

    void modeChanged(FileMode newMode, FileMode oldMode);
    void fileEdited(bool val);

    void editingSubModeChanged(EditingSubMode oldMode, EditingSubMode newMode);

private:
    FileMode mMode = FileMode::Editing;
    EditingSubMode mEditingMode = EditingSubMode::Default;

    NodeEditFactory *mCircuitFactory;
    CircuitListModel *mCircuitList;

    PanelItemFactory *mPanelItemFactory;
    PanelListModel *mPanelList;

    QHash<QString, AbstractSimulationObjectModel*> mObjectModels;

    SimulationObjectFactory *mObjectFactory;

    RemoteManager *mRemoteMgr = nullptr;
    SerialManager *mSerialMgr = nullptr;

    bool mFileWasEdited = false;

    QString mFilePath;
};

#endif // MODEMANAGER_H
