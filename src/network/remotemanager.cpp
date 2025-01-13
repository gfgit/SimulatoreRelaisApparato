#include "remotemanager.h"

#include "peerclient.h"
#include "peermanager.h"

#include "../views/modemanager.h"
#include "../objects/circuit_bridge/remotecircuitbridge.h"

RemoteManager::RemoteManager(ModeManager *mgr)
    : QObject(mgr)
{
    mPeerClient = new PeerClient(this);
    mPeerManager = mPeerClient->getPeerManager();
}

RemoteManager::~RemoteManager()
{
    mPeerClient->setCommunicationEnabled(false);

    delete mPeerClient;
    mPeerClient = nullptr;
    mPeerManager = nullptr;
}

ModeManager *RemoteManager::modeMgr() const
{
    return static_cast<ModeManager *>(parent());
}

QString RemoteManager::sessionName() const
{
    return mPeerManager->sessionName();
}

void RemoteManager::setSessionName(const QString &newSessionName)
{
    mPeerManager->setSessionName(newSessionName);
    modeMgr()->setFileEdited();
}

void RemoteManager::setOnline(bool val)
{
    mPeerClient->setCommunicationEnabled(val);
    if(val)
        setDiscoveryEnabled(true);
}

bool RemoteManager::isOnline() const
{
    return mPeerClient->isCommunicationEnabled();
}

void RemoteManager::setDiscoveryEnabled(bool val)
{
    if(!mPeerClient->isCommunicationEnabled())
        val = false;
    mPeerManager->setDiscoveryEnabled(val);
}

bool RemoteManager::isDiscoveryEnabled() const
{
    return mPeerManager->isDiscoveryEnabled();
}

bool RemoteManager::renameRemoteSession(const QString &fromName, const QString &toName)
{
    if(!mRemoteBridges.contains(fromName) || mRemoteBridges.contains(toName))
        return false;

    const auto vec = mRemoteBridges.take(fromName);

    for(RemoteCircuitBridge *bridge : vec)
        bridge->onRemoteSessionRenamed(toName);

    mRemoteBridges.insert(toName, vec);

    return true;
}

void RemoteManager::addRemoteBridge(RemoteCircuitBridge *bridge, const QString &peerSession)
{
    auto it = mRemoteBridges.find(peerSession);
    if(it == mRemoteBridges.end())
        it = mRemoteBridges.insert(peerSession, {});

    it.value().append(bridge);
}

void RemoteManager::removeRemoteBridge(RemoteCircuitBridge *bridge, const QString &peerSession)
{
    auto it = mRemoteBridges.find(peerSession);
    Q_ASSERT(it != mRemoteBridges.end());

    it.value().removeOne(bridge);
    if(it.value().isEmpty())
        mRemoteBridges.erase(it);
}
