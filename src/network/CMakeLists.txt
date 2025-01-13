# src/network

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

    PARENT_SCOPE
)
