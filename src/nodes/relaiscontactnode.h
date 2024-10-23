#ifndef RELAISCONTACTNODE_H
#define RELAISCONTACTNODE_H

#include "abstractcircuitnode.h"

class AbstractRelais;
class RelaisModel;

class RelaisContactNode : public AbstractCircuitNode
{
    Q_OBJECT
public:
    enum State
    {
        Down = 0,
        Up = 1,
        Middle
    };

    explicit RelaisContactNode(QObject *parent = nullptr);
    ~RelaisContactNode();

    QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    bool loadFromJSON(const QJsonObject& obj) override;
    void saveToJSON(QJsonObject& obj) const override;

    static constexpr QLatin1String NodeType = QLatin1String("relais_contact");
    QString nodeType() const override;

    AbstractRelais *relais() const;
    void setRelais(AbstractRelais *newRelais);

    State state() const;
    void setState(State newState);

    bool swapContactState() const;
    void setSwapContactState(bool newSwapContactState);

    bool flipContact() const;
    void setFlipContact(bool newFlipContact);

    bool hasCentralConnector() const;
    void setHasCentralConnector(bool newHasCentralConnector);

    RelaisModel *relaisModel() const;
    void setRelaisModel(RelaisModel *newRelaisModel);


signals:
    void stateChanged();
    void relayChanged(AbstractRelais *r);
    void shapeChanged();

private slots:
    void onRelaisStateChanged();

private:
    RelaisModel *mRelaisModel = nullptr;
    AbstractRelais *mRelais = nullptr;
    State mState = State::Middle;
    bool mFlipContact = false;
    bool mSwapContactState = false;
    bool mHasCentralConnector = true;
};

#endif // RELAISCONTACTNODE_H
