#ifndef LIGHTBULBNODE_H
#define LIGHTBULBNODE_H

#include "../abstractcircuitnode.h"

class LightBulbNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    explicit LightBulbNode(QObject *parent = nullptr);
    ~LightBulbNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    static constexpr QLatin1String NodeType = QLatin1String("light_bulb");
    QString nodeType() const override;
};

#endif // LIGHTBULBNODE_H
