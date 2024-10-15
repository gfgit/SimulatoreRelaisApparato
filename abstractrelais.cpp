#include "abstractrelais.h"

#include "nodes/relaiscontactnode.h"
#include "nodes/relaispowernode.h"

#include <QTimerEvent>

AbstractRelais::AbstractRelais(QObject *parent)
    : QObject{parent}
{

}

AbstractRelais::~AbstractRelais()
{
    auto powerNodes = mPowerNodes;
    for(RelaisPowerNode *p : powerNodes)
    {
        removePowerNode(p);
    }

    auto contactNodes = mContactNodes;
    for(RelaisContactNode *c : contactNodes)
    {
        removeContactNode(c);
    }

    killTimer(mTimerId);
    mTimerId = 0;
}

QString AbstractRelais::name() const
{
    return mName;
}

void AbstractRelais::setName(const QString &newName)
{
    if (mName == newName)
        return;
    mName = newName;
    emit nameChanged(mName);

    for(RelaisPowerNode *p : mPowerNodes)
    {
        p->setObjectName(mName);
    }

    for(RelaisContactNode *c : mContactNodes)
    {
        c->setObjectName(mName);
    }
}

double AbstractRelais::upSpeed() const
{
    return mUpSpeed;
}

void AbstractRelais::setUpSpeed(double newUpSpeed)
{
    mUpSpeed = newUpSpeed;
}

double AbstractRelais::downSpeed() const
{
    return mDownSpeed;
}

void AbstractRelais::setDownSpeed(double newDownSpeed)
{
    mDownSpeed = newDownSpeed;
}

void AbstractRelais::addPowerNode(RelaisPowerNode *p)
{
    Q_ASSERT(!mPowerNodes.contains(p));
    Q_ASSERT(!p->relais());

    mPowerNodes.append(p);
    p->setRelais(this);
    p->setObjectName(mName);

    if(p->hasCircuits())
    {
        powerNodeActivated(p);
    }
}

void AbstractRelais::removePowerNode(RelaisPowerNode *p)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->relais() == this);

    if(p->hasCircuits())
    {
        powerNodeDeactivated(p);
    }

    mPowerNodes.removeOne(p);
    p->setRelais(nullptr);
}

void AbstractRelais::addContactNode(RelaisContactNode *c)
{
    Q_ASSERT(!mContactNodes.contains(c));
    Q_ASSERT(!c->relais());

    mContactNodes.append(c);
    c->setRelais(this);
    c->setObjectName(mName);
}

void AbstractRelais::removeContactNode(RelaisContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->relais() == this);

    mContactNodes.removeOne(c);
    c->setRelais(nullptr);
}

void AbstractRelais::timerEvent(QTimerEvent *e)
{
    if(mTimerId && e->timerId() == mTimerId)
    {
        double newPosition = mPosition;
        if(mInternalState == State::GoingUp)
            newPosition += mUpSpeed;
        else if(mInternalState == State::GoingDown)
            newPosition -= mDownSpeed;
        else
        {
            killTimer(mTimerId);
            mTimerId = 0;
        }

        setPosition(newPosition);

        if((newPosition < 0.0) || (newPosition > 1.0))
        {
            mInternalState = mState;
            killTimer(mTimerId);
            mTimerId = 0;
        }

        return;
    }

    QObject::timerEvent(e);
}

void AbstractRelais::powerNodeActivated(RelaisPowerNode *p)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->mRelais == this);
    Q_ASSERT(mActivePowerNodes < mPowerNodes.size());

    mActivePowerNodes++;
    if(mActivePowerNodes == 1)
    {
        // Begin powering relais
        // TODO
        startMove(true);
    }
}

void AbstractRelais::powerNodeDeactivated(RelaisPowerNode *p)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->mRelais == this);
    Q_ASSERT(mActivePowerNodes > 0);

    mActivePowerNodes--;
    if(mActivePowerNodes == 0)
    {
        // End powering relais
        startMove(false);
    }
}

void AbstractRelais::setPosition(double newPosition)
{
    newPosition = qBound(0.0, newPosition, 1.0);

    if(qFuzzyCompare(mPosition, newPosition))
        return;

    const bool up = newPosition > mPosition;

    mPosition = newPosition;

    if(mPosition < 0.1)
        setState(State::Down);
    else if(mPosition > 0.9)
        setState(State::Up);
    else if(up)
        setState(State::GoingUp);
    else
        setState(State::GoingDown);
}

void AbstractRelais::startMove(bool up)
{
    mInternalState = up ? State::GoingUp : State::GoingDown;
    killTimer(mTimerId);
    mTimerId = startTimer(250);
}

AbstractRelais::State AbstractRelais::state() const
{
    return mState;
}

void AbstractRelais::setState(State newState)
{
    if (mState == newState)
        return;
    mState = newState;
    emit stateChanged(mState);
}
