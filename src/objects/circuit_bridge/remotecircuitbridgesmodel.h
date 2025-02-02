#ifndef REMOTE_CIRCUIT_BRIDGES_MODEL_H
#define REMOTE_CIRCUIT_BRIDGES_MODEL_H

#include "../abstractsimulationobjectmodel.h"

class RemoteCircuitBridgesModel : public AbstractSimulationObjectModel
{
    Q_OBJECT

public:
    enum ExtraColumns
    {
        RemoteSession = Columns::NCols,
        RemoteNode,
        NColsExtra
    };

    RemoteCircuitBridgesModel(ModeManager *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    // Custom remote circuit bridge specific data:
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
};

#endif // REMOTE_CIRCUIT_BRIDGES_MODEL_H
