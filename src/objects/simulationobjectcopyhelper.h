#ifndef SIMULATIONOBJECTCOPYHELPER_H
#define SIMULATIONOBJECTCOPYHELPER_H

#include <QHash>
#include <QStringList>

class ModeManager;
class AbstractSimulationObject;

class QJsonObject;

class SimulationObjectCopyHelper
{
public:
    static constexpr const QLatin1String CircuitMimeType = QLatin1String("application/x-simulatore-rele-circuits");

    static QJsonObject copyObjects(ModeManager *modeMgr,
                                   const QHash<QString, QStringList> &objMap);

    static QJsonObject copyObjects(ModeManager *modeMgr,
                                   const QVector<AbstractSimulationObject *> &objToCopy);

    static void pasteObjects(ModeManager *modeMgr,
                             const QJsonObject& objectPool);

    static void copyToClipboard(const QJsonObject& data);

    static bool getPasteDataFromClipboard(QJsonObject &data);
};

#endif // SIMULATIONOBJECTCOPYHELPER_H
