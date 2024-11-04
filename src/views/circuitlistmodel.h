#ifndef CIRCUITLISTMODEL_H
#define CIRCUITLISTMODEL_H

#include <QAbstractTableModel>

#include "../enums/filemodes.h"

class CircuitScene;
class AbstractNodeGraphItem;
class CableGraphItem;

class ModeManager;

class QJsonObject;

class CircuitListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        NameCol = 0,
        LongNameCol,
        NCols
    };

    explicit CircuitListModel(ModeManager *mgr, QObject *parent = nullptr);
    ~CircuitListModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    bool isNameAvailable(const QString& name) const;

    inline bool hasUnsavedChanges() const
    {
        return mHasUnsavedChanges;
    }

    void resetHasUnsavedChanges();

    CircuitScene *addCircuitScene(const QString& name);
    bool removeSceneAtRow(int row);

    void clear();

    inline ModeManager *modeMgr() const
    {
        return mModeMgr;
    }

    inline const QVector<CircuitScene *> getScenes() const
    {
        return mCircuitScenes;
    }

    inline CircuitScene *sceneAtRow(int row) const
    {
        return mCircuitScenes.value(row, nullptr);
    }

    bool loadFromJSON(const QJsonObject &obj);
    void saveToJSON(QJsonObject &obj) const;

signals:
    void nodeEditRequested(AbstractNodeGraphItem *item);
    void cableEditRequested(CableGraphItem *item);

private slots:
    void onSceneNameChanged(const QString& name, CircuitScene *scene);

    void setMode(FileMode newMode, FileMode oldMode);

private:
    friend class CircuitScene;
    void onSceneEdited();

private:
    QVector<CircuitScene *> mCircuitScenes;

    ModeManager *mModeMgr;

    bool mHasUnsavedChanges = false;
};

#endif // CIRCUITLISTMODEL_H
