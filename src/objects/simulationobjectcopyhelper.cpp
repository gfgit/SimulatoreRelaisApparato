#include "simulationobjectcopyhelper.h"

#include "../views/modemanager.h"

#include "abstractsimulationobject.h"
#include "abstractsimulationobjectmodel.h"

#include <QJsonObject>
#include <QJsonArray>

#include <QClipboard>
#include <QMimeData>
#include <QJsonDocument>

#include <QGuiApplication>

QJsonObject SimulationObjectCopyHelper::copyObjects(ModeManager *modeMgr, const QHash<QString, QStringList> &objMap)
{
    QVector<AbstractSimulationObject *> objectsToCheck;

    for(auto it = objMap.cbegin(); it != objMap.cend(); it++)
    {
        AbstractSimulationObjectModel *model = modeMgr->modelForType(it.key());
        if(!model)
            continue;

        for(const QString& objName : it.value())
        {
            AbstractSimulationObject *item = model->getObjectByName(objName);
            if(!item)
                continue;

            objectsToCheck.append(item);
        }
    }

    return copyObjects(modeMgr, objectsToCheck);
}

QJsonObject SimulationObjectCopyHelper::copyObjects(ModeManager *modeMgr, const QVector<AbstractSimulationObject *> &objToCopy)
{
    QVector<AbstractSimulationObject *> objectsToCheck = objToCopy;

    QSet<AbstractSimulationObject *> checkedObjects;
    QHash<QString, QVector<AbstractSimulationObject*>> objMap;

    while(!objectsToCheck.isEmpty())
    {
        AbstractSimulationObject *item = objectsToCheck.takeLast();
        checkedObjects.insert(item);

        QSet<AbstractSimulationObject *> otherItems;
        item->getReferencedObjects(otherItems);

        for(AbstractSimulationObject *other : std::as_const(otherItems))
        {
            if(checkedObjects.contains(other))
                continue;

            objectsToCheck.append(other);
        }

        auto it = objMap.find(item->getType());
        if(it == objMap.end())
            it = objMap.insert(item->getType(), {});

        it.value().append(item);
    }

    QJsonObject pool;

    for(auto it = objMap.cbegin(); it != objMap.cend(); it++)
    {
        AbstractSimulationObjectModel *model = modeMgr->modelForType(it.key());
        if(!model)
            continue;

        QJsonArray arr;

        for(AbstractSimulationObject *item : it.value())
        {
            QJsonObject obj;
            item->saveToJSON(obj);
            arr.append(obj);
        }

        QJsonObject modelObj;
        modelObj["model_type"] = it.key();
        modelObj["objects"] = arr;

        pool[it.key()] = modelObj;
    }

    return pool;
}

void SimulationObjectCopyHelper::pasteObjects(ModeManager *modeMgr, const QJsonObject &objectPool)
{
    // TODO: what to do in case of duplicate names?
    // Currently we are skipping them

    QJsonObject validPool;

    for(const QString& key : objectPool.keys())
    {
        const QJsonValue v = objectPool[key];
        if(!v.isObject())
            continue;

        const QJsonObject modelObj = v.toObject();
        if(modelObj["model_type"].toString() != key)
            continue; // Malformed data

        const QJsonArray arr = modelObj["objects"].toArray();

        AbstractSimulationObjectModel *model = modeMgr->modelForType(key);
        if(!model)
            continue;

        QJsonArray result;
        model->addObjectsFromArray(arr, result, LoadPhase::Creation);

        if(result.isEmpty())
            continue;

        validPool[key] = result;
    }

    for(const QString& key : validPool.keys())
    {
        const QJsonArray arr = validPool[key].toArray();

        AbstractSimulationObjectModel *model = modeMgr->modelForType(key);

        QJsonArray result;
        model->addObjectsFromArray(arr, result, LoadPhase::AllCreated);
    }
}

void SimulationObjectCopyHelper::copyToClipboard(const QJsonObject &data)
{
    QByteArray value = QJsonDocument(data).toJson(QJsonDocument::Compact);

    QMimeData *mime = new QMimeData;
    mime->setData(CircuitMimeType, value);

    QGuiApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

bool SimulationObjectCopyHelper::getPasteDataFromClipboard(QJsonObject &data)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if(!clipboard)
        return false;

    const QMimeData *mime = clipboard->mimeData(QClipboard::Clipboard);
    if(!mime)
        return false;

    QByteArray value = mime->data(CircuitMimeType);
    if(value.isEmpty())
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(value);
    if(doc.isNull())
        return false;

    data = doc.object();
    return true;
}
