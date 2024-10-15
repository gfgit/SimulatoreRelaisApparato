#ifndef POWERSOURCENODE_H
#define POWERSOURCENODE_H

#include "../abstractcircuitnode.h"

class PowerSourceNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit PowerSourceNode(QObject *parent = nullptr);

    virtual QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    bool getEnabled() const;
    void setEnabled(bool newEnabled);

signals:
    void enabledChanged(bool val);

private:
    bool enabled = false;
};

#endif // POWERSOURCENODE_H
