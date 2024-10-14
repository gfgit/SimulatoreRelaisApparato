#include "abstractrelais.h"

#include "nodes/relaiscontactnode.h"
#include "nodes/relaispowernode.h"

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
    Q_ASSERT(!p->relais());

    mPowerNodes.append(p);
    p->setRelais(this);

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
}

void AbstractRelais::removeContactNode(RelaisContactNode *c)
{
    Q_ASSERT(mContactNodes.contains(c));
    Q_ASSERT(c->relais() == this);

    mContactNodes.removeOne(c);
    c->setRelais(nullptr);
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
        setState(State::Up);
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
        setState(State::Down);
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
