#ifndef ACEIBUTTONNODE_H
#define ACEIBUTTONNODE_H

#include "abstractcircuitnode.h"

class ACEIButtonNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    enum State
    {
        Normal = 0,
        Pressed,
        Extracted
    };

    explicit ACEIButtonNode(QObject *parent = nullptr);
    ~ACEIButtonNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("acei_button");
    QString nodeType() const override;

    State state() const;
    void setState(State newState);

    bool flipContact() const;
    void setFlipContact(bool newFlipContact);

signals:
    void stateChanged();
    void shapeChanged();

private:
    State mState = State::Normal;
    bool mFlipContact = false;
};

#endif // ACEIBUTTONNODE_H
