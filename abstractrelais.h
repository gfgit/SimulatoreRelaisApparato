#ifndef ABSTRACTRELAIS_H
#define ABSTRACTRELAIS_H

#include <QObject>

class RelaisPowerNode;

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

    State state() const;
    void setState(State newState);

    QString name() const;

    void setName(const QString &newName);

    double upSpeed() const;
    void setUpSpeed(double newUpSpeed);

    double downSpeed() const;
    void setDownSpeed(double newDownSpeed);

    void addPowerNode(RelaisPowerNode *p);
    void removePowerNode(RelaisPowerNode *p);

signals:
    void nameChanged(const QString& name);
    void stateChanged(State s);

private:
    friend class RelaisPowerNode;
    void powerNodeActivated(RelaisPowerNode *p);
    void powerNodeDeactivated(RelaisPowerNode *p);

private:
    QString mName;
    State mState = State::Down;
    double mUpSpeed = 1.0;
    double mDownSpeed = 2.0;

    QVector<RelaisPowerNode *> mPowerNodes;
    int mActivePowerNodes = 0;
};

#endif // ABSTRACTRELAIS_H
