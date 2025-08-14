# src/circuits/graphs

set(SIMULATORE_RELAIS_SOURCES
    ${SIMULATORE_RELAIS_SOURCES}

    circuits/graphs/abstractnodegraphitem.cpp
    circuits/graphs/abstractnodegraphitem.h
    circuits/graphs/abstractdeviatorgraphitem.cpp
    circuits/graphs/abstractdeviatorgraphitem.h
    circuits/graphs/buttoncontactgraphitem.cpp
    circuits/graphs/buttoncontactgraphitem.h
    circuits/graphs/bifilarizatorgraphitem.cpp
    circuits/graphs/bifilarizatorgraphitem.h
    circuits/graphs/cablegraphitem.cpp
    circuits/graphs/cablegraphitem.h
    circuits/graphs/diodegraphitem.cpp
    circuits/graphs/diodegraphitem.h
    circuits/graphs/electromagnetgraphitem.cpp
    circuits/graphs/electromagnetgraphitem.h
    circuits/graphs/levercontactgraphitem.cpp
    circuits/graphs/levercontactgraphitem.h
    circuits/graphs/lightbulbgraphitem.cpp
    circuits/graphs/lightbulbgraphitem.h
    circuits/graphs/onoffgraphitem.cpp
    circuits/graphs/onoffgraphitem.h
    circuits/graphs/polarityinversiongraphitem.cpp
    circuits/graphs/polarityinversiongraphitem.h
    circuits/graphs/powersourcegraphitem.cpp
    circuits/graphs/powersourcegraphitem.h
    circuits/graphs/relaiscontactgraphitem.cpp
    circuits/graphs/relaiscontactgraphitem.h
    circuits/graphs/relaispowergraphitem.cpp
    circuits/graphs/relaispowergraphitem.h

    circuits/graphs/remotecablecircuitgraphitem.cpp
    circuits/graphs/remotecablecircuitgraphitem.h

    circuits/graphs/screenrelaiscontactgraphitem.cpp
    circuits/graphs/screenrelaiscontactgraphitem.h
    circuits/graphs/screenrelaispowergraphitem.cpp
    circuits/graphs/screenrelaispowergraphitem.h

    circuits/graphs/simplenodegraphitem.cpp
    circuits/graphs/simplenodegraphitem.h
    circuits/graphs/simpleactivationgraphitem.cpp
    circuits/graphs/simpleactivationgraphitem.h

    circuits/graphs/soundcircuitgraphitem.cpp
    circuits/graphs/soundcircuitgraphitem.h

    circuits/graphs/special/aceibuttongraphitem.cpp
    circuits/graphs/special/aceibuttongraphitem.h

    circuits/graphs/special/aceilevergraphitem.cpp
    circuits/graphs/special/aceilevergraphitem.h

    circuits/graphs/special/acesasiblevergraphitem.cpp
    circuits/graphs/special/acesasiblevergraphitem.h

    circuits/graphs/transformergraphitem.cpp
    circuits/graphs/transformergraphitem.h

    circuits/graphs/traintasticsensorcontactgraphitem.cpp
    circuits/graphs/traintasticsensorcontactgraphitem.h

    PARENT_SCOPE
)
