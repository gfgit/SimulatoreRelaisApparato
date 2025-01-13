#ifndef REMOTEMANAGER_H
#define REMOTEMANAGER_H

#include <QObject>

#include <QHash>

class ModeManager;

class PeerClient;
class PeerManager;
class PeerConnection;

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

    void onLocalBridgeModeChanged(quint64 peerSessionId, quint64 peerNodeId,
                                  qint8 mode, qint8 pole);

    void onRemoteBridgeModeChanged(quint64 peerSessionId, quint64 localNodeId,
                                   qint8 mode, qint8 pole);

    inline bool isSessionReferenced(const QString& name) const
    {
        return mRemoteBridges.contains(name);
    }

    struct BridgeListItem
    {
        quint64 peerNodeId;
        QString peerNodeName;
        QString localNodeName;
    };

    void onRemoteBridgeListReceived(PeerConnection *conn, const QVector<BridgeListItem>& list);

    struct BridgeResponse
    {
        QVector<quint64> failedIds;
        QHash<quint64, quint64> newMappings;
    };
    void onRemoteBridgeResponseReceived(PeerConnection *conn, const BridgeResponse& msg);

signals:
    void networkStateChanged();

private:
    friend class PeerClient;
    void addConnection(PeerConnection *conn);
    void removeConnection(PeerConnection *conn);

    void sendBridgesTo(PeerConnection *conn);

private:
    PeerClient *mPeerClient = nullptr;
    PeerManager *mPeerManager = nullptr;
    QHash<QString, QVector<RemoteCircuitBridge *>> mRemoteBridges;

    QHash<quint64, PeerConnection *> mConnections;
};

#endif // REMOTEMANAGER_H
