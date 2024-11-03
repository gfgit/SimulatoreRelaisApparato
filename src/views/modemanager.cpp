#include "modemanager.h"

#include "../nodes/edit/nodeeditfactory.h"

#include "circuitlistmodel.h"

#include "../objects/relaismodel.h"

#include <QJsonObject>

ModeManager::ModeManager(QObject *parent)
    : QObject{parent}
{
    mRelaisModel = new RelaisModel(this, this);

    mCircuitFactory = new NodeEditFactory(this);
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
    mRelaisModel->setHasUnsavedChanges(false);

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
