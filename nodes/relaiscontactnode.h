#ifndef RELAISCONTACTNODE_H
#define RELAISCONTACTNODE_H

#include "../abstractcircuitnode.h"

class AbstractRelais;

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

    State state() const;
    void setState(State newState);

signals:
    void stateChanged();
    void relayChanged();

private slots:
    void onRelaisStateChanged();

private:
    friend class AbstractRelais;
    void setRelais(AbstractRelais *newRelais);

private:
    AbstractRelais *mRelais = nullptr;
    State mState = State::Middle;
};

#endif // RELAISCONTACTNODE_H
