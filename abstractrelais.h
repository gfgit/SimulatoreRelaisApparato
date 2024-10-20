#ifndef ABSTRACTRELAIS_H
#define ABSTRACTRELAIS_H

#include <QObject>

class RelaisPowerNode;
class RelaisContactNode;

class AbstractRelais : public QObject
{
    Q_OBJECT
public:

    enum class State
    {
        Up = 0,
        Down = 1,
        GoingUp = 2,
        GoingDown = 3
    };

    explicit AbstractRelais(QObject *parent = nullptr);
    ~AbstractRelais();

    State state() const;
    void setState(State newState);

    QString name() const;

    void setName(const QString &newName);

    double upSpeed() const;
    void setUpSpeed(double newUpSpeed);

    double downSpeed() const;
    void setDownSpeed(double newDownSpeed);

    void timerEvent(QTimerEvent *e) override;

signals:
    void nameChanged(AbstractRelais *self, const QString& name);
    void stateChanged(AbstractRelais *self, State s);

private:
    friend class RelaisPowerNode;
    void addPowerNode(RelaisPowerNode *p);
    void removePowerNode(RelaisPowerNode *p);

    friend class RelaisContactNode;
    void addContactNode(RelaisContactNode *c);
    void removeContactNode(RelaisContactNode *c);

    void powerNodeActivated(RelaisPowerNode *p);
    void powerNodeDeactivated(RelaisPowerNode *p);

    void setPosition(double newPosition);
    void startMove(bool up);

private:
    QString mName;
    State mState = State::Down;
    State mInternalState = State::Down;

    // Steps per 250ms (Speed of 0.25 means it goes up in 1 sec)
    double mUpSpeed = 0.18;
    double mDownSpeed = 0.25;
    double mPosition = 0.0;
    int mTimerId = 0;

    QVector<RelaisPowerNode *> mPowerNodes;
    int mActivePowerNodes = 0;

    QVector<RelaisContactNode *> mContactNodes;
};

#endif // ABSTRACTRELAIS_H
