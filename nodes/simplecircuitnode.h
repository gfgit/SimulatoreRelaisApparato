#ifndef SIMPLECIRCUITNODE_H
#define SIMPLECIRCUITNODE_H

#include "../abstractcircuitnode.h"

class SimpleCircuitNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit SimpleCircuitNode(QObject *parent = nullptr);

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    void addCircuit(ClosedCircuit *circuit) override;
    void removeCircuit(ClosedCircuit *circuit) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("simple_node");
    QString nodeType() const override;

    inline int disabledContact() const
    {
        return mDisabledContact;
    }

    void setDisabledContact(int val);

    inline bool isContactEnabled(int nodeContact) const
    {
        if(nodeContact == 0)
            return true; // Common is always enables
        return nodeContact != mDisabledContact;
    }

    inline bool hasCircuit(int nodeContact) const
    {
        Q_ASSERT(nodeContact >= 0 && nodeContact < 4);
        if(nodeContact == 0)
            return hasCircuits(); // Common
        return mCircuitCount[nodeContact - 1] > 0;
    }

signals:
    void disabledContactChanged();

private:
    int mDisabledContact = 0; // All enabled
    int mCircuitCount[3] = {0, 0, 0}; // Circuits on every branch
};

#endif // SIMPLECIRCUITNODE_H
