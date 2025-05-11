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

#include "../panels/edit/panelitemfactory.h"
#include "../panels/edit/standardpanelitemtypes.h"
#include "../panels/view/panellistmodel.h"

#include "../objects/simulationobjectfactory.h"
#include "../objects/standardobjecttypes.h"
#include "../objects/abstractsimulationobjectmodel.h"

#include "../enums/loadphase.h"

#include "../network/remotemanager.h"
#include "../serial/serialmanager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

QJsonObject convertFileFormatBetaToV1(const QJsonObject& origFile)
{

    QJsonObject objects;

    // Port Relais
    QJsonObject oldRelais = origFile.value("relais").toObject();
    const QJsonArray relaisArr = oldRelais.value("relais").toArray();
    QJsonArray newRelaisArr;

    for(const QJsonValue& v : relaisArr)
    {
        QJsonObject relay = v.toObject();
        relay["type"] = "abstract_relais";
        relay["description"] = QString();
        relay["interfaces"] = QJsonObject();
        newRelaisArr.append(relay);
    }

    QJsonObject newRelaisModel;
    newRelaisModel["model_type"] = "abstract_relais";
    newRelaisModel["objects"] = newRelaisArr;

    objects["abstract_relais"] = newRelaisModel;

    // Port levers
    QJsonObject oldLevers = origFile.value("levers").toObject();
    const QJsonArray leversArr = oldLevers.value("levers").toArray();
    QJsonArray newLeversArr;

    for(const QJsonValue& v : leversArr)
    {
        QJsonObject oldLever = v.toObject();

        QJsonObject lever;
        lever["type"] = "acei_lever";
        lever["description"] = QString();
        lever["name"] = oldLever.value("name");

        QJsonObject leverInterface = oldLever;
        leverInterface.remove("name");

        QJsonObject interfaces;
        interfaces["lever"] = leverInterface;

        lever["interfaces"] = interfaces;
        newLeversArr.append(lever);
    }

    QJsonObject newLeversModel;
    newLeversModel["model_type"] = "acei_lever";
    newLeversModel["objects"] = newLeversArr;

    objects["acei_lever"] = newLeversModel;

    // Light Bulbs need to be created
    QStringList lightBulbNames;

    QJsonObject circuits = origFile["circuits"].toObject();
    QJsonArray newScenes;

    QJsonArray scenes = QJsonValueRef(circuits["scenes"]).toArray();
    for(const QJsonValue v : scenes)
    {
        QJsonObject scene = v.toObject();
        QJsonArray nodes;
        for(QJsonValueRef nodeVal : scene["nodes"].toArray())
        {
            QJsonObject node = nodeVal.toObject();
            if(node.value("type") == "lever_contact" || node.value("type") == "acei_lever")
            {
                node["lever_type"] = "acei_lever";
            }
            else if(node.value("type") == "light_bulb")
            {
                node["type"] = "light_bulb_activation";
                node["object"] = node.value("name").toString();
                lightBulbNames.append(node.value("name").toString());
            }
            nodes.append(node);
        }
        scene["nodes"] = nodes;
        newScenes.append(scene);
    }

    // Create light bulbs
    QJsonArray newLightArr;

    for(const QString& name : lightBulbNames)
    {
        QJsonObject light;
        light["type"] = "light_bulb";
        light["description"] = QString();
        light["name"] = name;

        QJsonObject interfaces;
        light["interfaces"] = interfaces;
        newLightArr.append(light);
    }

    QJsonObject newLightModel;
    newLightModel["model_type"] = "light_bulb";
    newLightModel["objects"] = newLightArr;

    objects["light_bulb"] = newLightModel;

    // Save new file
    QJsonObject newFile;
    newFile["file_version"] = ModeManager::FileVersion::V1;
    newFile["objects"] = objects;


    QJsonObject newCircuits;
    newCircuits["scenes"] = newScenes;
    newFile["circuits"] = newCircuits;

    return newFile;
}

QJsonObject replaceACESasibType(const QJsonObject origFile)
{
    QJsonDocument doc(origFile);
    QByteArray data = doc.toJson();
    data.replace("ace_sasib_lever_5", "ace_sasib_lever_2");
    data.replace("ace_sasib_lever_7", "ace_sasib_lever_3");
    doc = QJsonDocument::fromJson(data);
    return doc.object();
}

QJsonObject convertFileFormatV1ToV2(const QJsonObject& origFileOld)
{
    const QJsonObject origFile = replaceACESasibType(origFileOld);

    QJsonObject circuits = origFile.value("circuits").toObject();
    const QJsonArray scenes = circuits.value("scenes").toArray();

    QJsonArray newScenes;
    for(const QJsonValue& v : scenes)
    {
        QJsonObject scene = v.toObject();

        QJsonArray nodes;
        for(QJsonValueRef nodeVal : scene["nodes"].toArray())
        {
            QJsonObject node = nodeVal.toObject();
            const QString nodeType = node.value("type").toString();
            if(nodeType == "button_contact")
            {
                // Convert button contact positions
                // Normal 0 -> 1, Pressed 1 -> 0
                const QString fmt("state_%1_contact_%2");
                bool butState[2][2] = {{false, false}, {false, false}};
                for(qint64 i = 0; i <= 1; i++)
                {
                    butState[i][0] = node.value(fmt.arg(i).arg(0)).toBool();
                    butState[i][1] = node.value(fmt.arg(i).arg(1)).toBool();
                }

                // Swap
                std::swap(butState[0], butState[1]);

                for(qint64 i = 0; i <= 1; i++)
                {
                    node[fmt.arg(i).arg(0)] = butState[i][0];
                    node[fmt.arg(i).arg(1)] = butState[i][1];
                }
            }
            else if(nodeType == "screen_relais_contact")
            {
                // Invert contact state
                bool swapState = node.value("swap_state").toBool();
                node["swap_state"] = !swapState;
            }

            nodes.append(node);
        }
        scene["nodes"] = nodes;

        newScenes.append(scene);
    }

    QJsonObject objects = origFile.value("objects").toObject();

    {
        // Add MechanicalInterface to buttons
        QJsonObject butModel = objects.value("generic_button").toObject();
        const QJsonArray arr = butModel.value("objects").toArray();
        QJsonArray newButtons;

        for(const QJsonValue& v : arr)
        {
            QJsonObject buttonObj = v.toObject();

            QJsonObject interfaces = buttonObj.value("interfaces").toObject();
            const QJsonObject butIface = interfaces.value("button").toObject();

            QJsonObject mechIface;
            mechIface["pos_min"] = butIface.value("can_press").toBool(true) ? 0 : 1;
            mechIface["pos_max"] = butIface.value("can_extract").toBool(false) ? 2 : 1;
            interfaces["mechanical"] = mechIface;
            buttonObj["interfaces"] = interfaces;

            newButtons.append(buttonObj);
        }

        butModel["objects"] = newButtons;
        objects["generic_button"] = butModel;
    }

    {
        // Swap condition sets for ACE Lever 3 pos
        QJsonObject leverModel = objects.value("ace_sasib_lever_3").toObject();
        const QJsonArray arr = leverModel.value("objects").toArray();
        QJsonArray newLevers;

        for(const QJsonValue& v : arr)
        {
            QJsonObject leverObj = v.toObject();

            QJsonObject interfaces = leverObj.value("interfaces").toObject();
            QJsonObject mechIface = interfaces.value("mechanical").toObject();

            QJsonArray conditions = mechIface.value("conditions").toArray();
            if(conditions.size() == 2)
            {
                // Swap
                QJsonArray newConditions;
                newConditions.append(conditions.at(1).toObject());
                newConditions.append(conditions.at(0).toObject());
                mechIface["conditions"] = newConditions;
            }

            interfaces["mechanical"] = mechIface;
            leverObj["interfaces"] = interfaces;

            newLevers.append(leverObj);
        }

        leverModel["objects"] = newLevers;
        objects["ace_sasib_lever_3"] = leverModel;
    }

    // Save new file
    QJsonObject newFile = origFile;
    newFile["file_version"] = ModeManager::FileVersion::V2;

    QJsonObject newCircuits;
    newCircuits["scenes"] = newScenes;
    newFile["circuits"] = newCircuits;
    newFile["objects"] = objects;

    return newFile;
}

QJsonObject convertOldFileFormat(const QJsonObject& origFile)
{
    QJsonObject rootObj = origFile;

    if(rootObj.value("file_version") == ModeManager::FileVersion::Beta)
    {
        // Old file, try to convert it to V1
        rootObj = convertFileFormatBetaToV1(rootObj);
    }

    if(rootObj.value("file_version") == ModeManager::FileVersion::V1)
    {
        // V1 file, try to convert it to V2
        rootObj = convertFileFormatV1ToV2(rootObj);
    }

    return rootObj;
}

ModeManager::ModeManager(QObject *parent)
    : QObject{parent}
{
    // Circuits
    mCircuitFactory = new NodeEditFactory(this);
    StandardNodeTypes::registerTypes(mCircuitFactory);

    mCircuitList = new CircuitListModel(this, this);

    // Panels
    mPanelItemFactory = new PanelItemFactory(this);
    StandardPanelItemTypes::registerTypes(mPanelItemFactory);

    mPanelList = new PanelListModel(this, this);

    // Objects
    mObjectFactory = new SimulationObjectFactory;
    StandardObjectTypes::registerTypes(mObjectFactory);

    for(const QString& objType : mObjectFactory->getRegisteredTypes())
    {
        AbstractSimulationObjectModel *model =
                mObjectFactory->createModel(this, objType);

        model->setParent(this); // Ownership for cleanup
        mObjectModels.insert(objType, model);
    }

    mRemoteMgr = new RemoteManager(this);
    mSerialMgr = new SerialManager(this);
}

ModeManager::~ModeManager()
{
    // Disable network communication
    mRemoteMgr->setOnline(false);

    mSerialMgr->disconnectAllDevices();

    // Delete circuits and factory
    // before objects
    delete mCircuitList;
    mCircuitList = nullptr;

    delete mCircuitFactory;
    mCircuitFactory = nullptr;

    delete mPanelList;
    mPanelList = nullptr;

    delete mPanelItemFactory;
    mPanelItemFactory = nullptr;

    // Delete objects and factory
    qDeleteAll(mObjectModels);
    mObjectModels.clear();

    // Delete remote connections AFTER deleting objects
    delete mRemoteMgr;
    mRemoteMgr = nullptr;

    delete mSerialMgr;
    mSerialMgr = nullptr;

    // Delete object factory as last
    delete mObjectFactory;
    mObjectFactory = nullptr;
}

void ModeManager::setMode(FileMode newMode)
{
    if (mMode == newMode)
        return;

    const FileMode oldMode = mMode;

    if(oldMode == FileMode::Editing)
    {
        // Reset sub editing mode to default
        setEditingSubMode(EditingSubMode::Default);
    }

    if(newMode != FileMode::Simulation)
    {
        // Stop network connections if not in Simulation mode
        mRemoteMgr->setOnline(false);

        mSerialMgr->disconnectAllDevices();
    }
    else if(oldMode != FileMode::Simulation)
    {
        mSerialMgr->rescanPorts();
    }

    mMode = newMode;
    emit modeChanged(mMode, oldMode);

    // Let widgets receive mode change first, then update all other scenes
    mCircuitList->setMode(mMode, oldMode);

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
    mPanelList->resetHasUnsavedChanges();

    for(auto model : mObjectModels)
        model->resetHasUnsavedChanges();

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

PanelItemFactory *ModeManager::panelFactory() const
{
    return mPanelItemFactory;
}

PanelListModel *ModeManager::panelList() const
{
    return mPanelList;
}

bool ModeManager::loadFromJSON(const QJsonObject &obj)
{
    // Temporarily ignore modified scenes
    setMode(FileMode::LoadingFile);

    QJsonObject rootObj = obj;
    if(obj.value("file_version") != FileVersion::Current)
    {
        // Old file, try to convert it
        rootObj = convertOldFileFormat(obj);
    }

    mRemoteMgr->setSessionName(obj.value("session_name").toString());

    for(auto model : mObjectModels)
    {
        model->clear();
    }

    const QJsonObject pool = rootObj.value("objects").toObject();
    for(auto model : mObjectModels)
    {
        const QJsonObject modelObj = pool.value(model->getObjectType()).toObject();
        model->loadFromJSON(modelObj, LoadPhase::Creation);
    }

    for(auto model : mObjectModels)
    {
        const QJsonObject modelObj = pool.value(model->getObjectType()).toObject();
        model->loadFromJSON(modelObj, LoadPhase::AllCreated);
    }

    QJsonObject circuits = rootObj.value("circuits").toObject();
    mCircuitList->loadFromJSON(circuits);

    QJsonObject panels = rootObj.value("panels").toObject();
    mPanelList->loadFromJSON(panels);

    resetFileEdited();

    // Turn on power sources and stuff
    setMode(FileMode::Simulation);

    return true;
}

void ModeManager::saveToJSON(QJsonObject &obj) const
{
    QJsonObject circuits;
    mCircuitList->saveToJSON(circuits);

    QJsonObject panels;
    mPanelList->saveToJSON(panels);

    QJsonObject pool;
    for(auto model : mObjectModels)
    {
        QJsonObject modelObj;
        model->saveToJSON(modelObj);
        pool[model->getObjectType()] = modelObj;
    }

    obj["file_version"] = FileVersion::Current;

    obj["session_name"] = mRemoteMgr->sessionName();

    obj["objects"] = pool;

    obj["circuits"] = circuits;
    obj["panels"] = panels;
}

void ModeManager::clearAll()
{
    setMode(FileMode::LoadingFile);

    mCircuitList->clear();
    mPanelList->clear();

    for(auto model : mObjectModels)
        model->clear();

    mRemoteMgr->setSessionName(QString());

    resetFileEdited();

    // Wait for new items
    setMode(FileMode::Editing);
}

EditingSubMode ModeManager::editingSubMode() const
{
    return mEditingMode;
}

void ModeManager::setEditingSubMode(EditingSubMode newEditingMode)
{
    if(mEditingMode == newEditingMode)
        return;

    EditingSubMode oldMode = mEditingMode;
    mEditingMode = newEditingMode;
    emit editingSubModeChanged(oldMode, mEditingMode);
}

SimulationObjectFactory *ModeManager::objectFactory() const
{
    return mObjectFactory;
}

AbstractSimulationObjectModel *ModeManager::modelForType(const QString &objType) const
{
    return mObjectModels.value(objType, nullptr);
}

QString ModeManager::filePath() const
{
    return mFilePath;
}

void ModeManager::setFilePath(const QString &newFilePath, bool newFile)
{
    const QString oldFile = mFilePath;

    mFilePath = newFilePath;

    emit fileChanged(mFilePath, newFile ? QString() : oldFile);
}
