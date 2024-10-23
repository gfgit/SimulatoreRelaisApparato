#ifndef RELAISPOWERNODE_H
#define RELAISPOWERNODE_H

#include "abstractcircuitnode.h"

class AbstractRelais;
class RelaisModel;

class RelaisPowerNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit RelaisPowerNode(QObject *parent = nullptr);
    ~RelaisPowerNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ElectricCircuit *circuit) override;
    void removeCircuit(ElectricCircuit *circuit, const NodeOccurences& items) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("relais_power");
    QString nodeType() const override;

    AbstractRelais *relais() const;
    void setRelais(AbstractRelais *newRelais);

    RelaisModel *relaisModel() const;
    void setRelaisModel(RelaisModel *newRelaisModel);

signals:
    void relayChanged(AbstractRelais *r);

private:
    AbstractRelais *mRelais = nullptr;
    RelaisModel *mRelaisModel = nullptr;
};

#endif // RELAISPOWERNODE_H
