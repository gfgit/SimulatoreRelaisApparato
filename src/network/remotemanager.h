#ifndef REMOTEMANAGER_H
#define REMOTEMANAGER_H

#include <QObject>

#include <QHash>

class ModeManager;

class PeerClient;
class PeerManager;
class RemoteCircuitBridge;

class RemoteManager : public QObject
{
    Q_OBJECT
public:
    explicit RemoteManager(ModeManager *mgr);
    ~RemoteManager();

    ModeManager *modeMgr() const;

    QString sessionName() const;
    void setSessionName(const QString &newSessionName);

    void setOnline(bool val);
    bool isOnline() const;

    void setDiscoveryEnabled(bool val);
    bool isDiscoveryEnabled() const;

    bool renameRemoteSession(const QString &fromName, const QString &toName);

    void addRemoteBridge(RemoteCircuitBridge *bridge, const QString& peerSession);
    void removeRemoteBridge(RemoteCircuitBridge *bridge, const QString& peerSession);

    inline bool isSessionReferenced(const QString& name) const
    {
        return mRemoteBridges.contains(name);
    }

signals:
    void networkStateChanged();

private:
    PeerClient *mPeerClient = nullptr;
    PeerManager *mPeerManager = nullptr;
    QHash<QString, QVector<RemoteCircuitBridge *>> mRemoteBridges;

};

#endif // REMOTEMANAGER_H
