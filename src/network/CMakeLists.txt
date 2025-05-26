# src/network

add_subdirectory(view)

set(SIMULATORE_RELAIS_SOURCES
    ${SIMULATORE_RELAIS_SOURCES}

    network/peerconnection.cpp
    network/peerconnection.h
    network/peermanager.cpp
    network/peermanager.h
    network/peerclient.cpp
    network/peerclient.h
    network/peerserver.cpp
    network/peerserver.h
    network/remotemanager.cpp
    network/remotemanager.h
    network/remotesession.cpp
    network/remotesession.h
    network/remotesessionsmodel.cpp
    network/remotesessionsmodel.h
    network/replicaobjectmanager.cpp
    network/replicaobjectmanager.h
    network/replicasmodel.cpp
    network/replicasmodel.h

    PARENT_SCOPE
)
