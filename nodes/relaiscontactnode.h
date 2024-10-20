#ifndef RELAISCONTACTNODE_H
#define RELAISCONTACTNODE_H

#include "../abstractcircuitnode.h"

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

    virtual QVector<CableItem> getActiveConnections(CableItem source, bool invertDir = false) override;

    AbstractRelais *relais() const;
    void setRelais(AbstractRelais *newRelais);

    State state() const;
    void setState(State newState);

    bool flipContact() const;
    void setFlipContact(bool newFlipContact);

    bool swapContactState() const;
    void setSwapContactState(bool newSwapContactState);

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
};

#endif // RELAISCONTACTNODE_H
