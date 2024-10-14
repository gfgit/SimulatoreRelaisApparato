#include "abstractrelais.h"

#include "nodes/relaispowernode.h"

AbstractRelais::AbstractRelais(QObject *parent)
    : QObject{parent}
{

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
    Q_ASSERT(!p->mRelais);

    mPowerNodes.append(p);
    p->mRelais = this;

    if(p->hasCircuits())
    {
        powerNodeActivated(p);
    }
}

void AbstractRelais::removePowerNode(RelaisPowerNode *p)
{
    Q_ASSERT(mPowerNodes.contains(p));
    Q_ASSERT(p->mRelais == this);

    if(p->hasCircuits())
    {
        powerNodeDeactivated(p);
    }

    mPowerNodes.removeOne(p);
    p->mRelais = nullptr;
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
        // TODO
    }
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
