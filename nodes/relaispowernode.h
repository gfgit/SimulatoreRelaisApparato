#ifndef RELAISPOWERNODE_H
#define RELAISPOWERNODE_H

#include "../abstractcircuitnode.h"

class AbstractRelais;
class RelaisModel;

class RelaisPowerNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit RelaisPowerNode(QObject *parent = nullptr);
    ~RelaisPowerNode();

    virtual QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    virtual void addCircuit(ClosedCircuit *circuit) override;
    virtual void removeCircuit(ClosedCircuit *circuit) override;



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
