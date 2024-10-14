#ifndef ONOFFSWITCHNODE_H
#define ONOFFSWITCHNODE_H

#include "../abstractcircuitnode.h"

class OnOffSwitchNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit OnOffSwitchNode(QObject *parent = nullptr);

    virtual QVector<CableItem> getConnections(CableItem source, bool invertDir = false) override;

    bool isOn() const;
    void setOn(bool newOn);

signals:
    void isOnChanged(bool on);

private:
    bool m_isOn = false;
};

#endif // ONOFFSWITCHNODE_H
