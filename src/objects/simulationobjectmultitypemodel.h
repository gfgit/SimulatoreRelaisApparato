#ifndef SIMULATIONOBJECTMULTITYPEMODEL_H
#define SIMULATIONOBJECTMULTITYPEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class AbstractSimulationObject;
class AbstractSimulationObjectModel;
class ModeManager;

class SimulationObjectMultiTypeModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SimulationObjectMultiTypeModel(ModeManager *mgr, const QStringList& types,
                                   QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    AbstractSimulationObject *objectAt(int row) const;

private slots:
    void onBeginReset();
    void onResetEnd();

private:
    AbstractSimulationObjectModel* getSourceModel(int row, int& outSourceRow) const;

private:
    ModeManager *modeMgr;
    QVector<AbstractSimulationObjectModel *> mModels;

    int mResetCount = 0;
    int mCachedRowCount = 0;
};

#endif // SIMULATIONOBJECTMULTITYPEMODEL_H
