# src/circuits/graphs

set(PROJECT_SOURCES
    ${PROJECT_SOURCES}

    circuits/graphs/abstractnodegraphitem.cpp
    circuits/graphs/abstractnodegraphitem.h
    circuits/graphs/abstractdeviatorgraphitem.cpp
    circuits/graphs/abstractdeviatorgraphitem.h
    circuits/graphs/aceibuttongraphitem.cpp
    circuits/graphs/aceibuttongraphitem.h
    circuits/graphs/cablegraphitem.cpp
    circuits/graphs/cablegraphitem.h
    circuits/graphs/levercontactgraphitem.cpp
    circuits/graphs/levercontactgraphitem.h
    circuits/graphs/lightbulbgraphitem.cpp
    circuits/graphs/lightbulbgraphitem.h
    circuits/graphs/onoffgraphitem.cpp
    circuits/graphs/onoffgraphitem.h
    circuits/graphs/powersourcegraphitem.cpp
    circuits/graphs/powersourcegraphitem.h
    circuits/graphs/relaiscontactgraphitem.cpp
    circuits/graphs/relaiscontactgraphitem.h
    circuits/graphs/relaispowergraphitem.cpp
    circuits/graphs/relaispowergraphitem.h
    circuits/graphs/simplenodegraphitem.cpp
    circuits/graphs/simplenodegraphitem.h

    circuits/graphs/special/aceilevergraphitem.cpp
    circuits/graphs/special/aceilevergraphitem.h

    PARENT_SCOPE
)
