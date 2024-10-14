#ifndef RELAISPOWERNODE_H
#define RELAISPOWERNODE_H

#include "../abstractcircuitnode.h"

class AbstractRelais;

class RelaisPowerNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit RelaisPowerNode(QObject *parent = nullptr);
    ~RelaisPowerNode();

    virtual QVector<CableItem> getConnections(CableItem source, bool invertDir = false) override;

    virtual void addCircuit(ClosedCircuit *circuit) override;
    virtual void removeCircuit(ClosedCircuit *circuit) override;

    AbstractRelais *relais() const;

signals:
    void relayChanged();

private:
    friend class AbstractRelais;
    void setRelais(AbstractRelais *newRelais);

private:
    AbstractRelais *mRelais = nullptr;
};

#endif // RELAISPOWERNODE_H
