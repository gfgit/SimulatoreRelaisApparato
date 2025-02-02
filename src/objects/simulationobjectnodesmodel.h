#ifndef SIMULATION_OBJECT_NODES_MODEL_H
#define SIMULATION_OBJECT_NODES_MODEL_H

#include <QAbstractTableModel>
#include <QVector>

class AbstractSimulationObject;
class AbstractNodeGraphItem;

class ViewManager;

class SimulationObjectNodesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Columns
    {
        NodeTypeCol = 0,
        SceneNameCol,
        NCols
    };

    explicit SimulationObjectNodesModel(ViewManager *viewMgr,
                                        QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    AbstractSimulationObject *getObject() const;
    void setObject(AbstractSimulationObject *newObject);

    inline AbstractNodeGraphItem *itemAt(int row) const
    {
        return mItems.value(row, nullptr);
    }

private slots:
    void refreshNodeList();
    void onObjectDestroyed();

private:
    ViewManager *mViewMgr = nullptr;

    AbstractSimulationObject *mObject = nullptr;
    QVector<AbstractNodeGraphItem *> mItems;
};

#endif // SIMULATION_OBJECT_NODES_MODEL_H
