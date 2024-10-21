#ifndef SIMPLECIRCUITNODE_H
#define SIMPLECIRCUITNODE_H

#include "../abstractcircuitnode.h"

class SimpleCircuitNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit SimpleCircuitNode(QObject *parent = nullptr);

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

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

signals:
    void disabledContactChanged();

private:
    int mDisabledContact = 0; // All enabled
};

#endif // SIMPLECIRCUITNODE_H
