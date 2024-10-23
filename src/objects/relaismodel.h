#ifndef RELAISMODEL_H
#define RELAISMODEL_H

#include <QAbstractListModel>

#include <QVector>

class AbstractRelais;
class QJsonObject;

class RelaisModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RelaisModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    void addRelay(AbstractRelais *r);
    void removeRelay(AbstractRelais *r);

    AbstractRelais *relayAt(int row) const;

    AbstractRelais *getRelay(const QString& name);

    void clear();
    bool loadFromJSON(const QJsonObject& obj);
    void saveToJSON(QJsonObject& obj) const;

private slots:
    void onRelayChanged(AbstractRelais *r);
    void onRelayDestroyed(QObject *obj);

private:
    QVector<AbstractRelais *> mRelais;
};

#endif // RELAISMODEL_H
