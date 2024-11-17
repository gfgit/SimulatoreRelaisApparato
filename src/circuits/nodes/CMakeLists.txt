# src/circuits/nodes

set(PROJECT_SOURCES
    ${PROJECT_SOURCES}

    circuits/nodes/abstractcircuitnode.cpp
    circuits/nodes/abstractcircuitnode.h
    circuits/nodes/abstractdeviatornode.cpp
    circuits/nodes/abstractdeviatornode.h
    circuits/nodes/aceibuttonnode.cpp
    circuits/nodes/aceibuttonnode.h
    circuits/nodes/circuitcable.cpp
    circuits/nodes/circuitcable.h
    circuits/nodes/levercontactnode.cpp
    circuits/nodes/levercontactnode.h
    circuits/nodes/lightbulbnode.cpp
    circuits/nodes/lightbulbnode.h
    circuits/nodes/onoffswitchnode.cpp
    circuits/nodes/onoffswitchnode.h
    circuits/nodes/polarityinversionnode.cpp
    circuits/nodes/polarityinversionnode.h
    circuits/nodes/powersourcenode.cpp
    circuits/nodes/powersourcenode.h
    circuits/nodes/relaiscontactnode.cpp
    circuits/nodes/relaiscontactnode.h
    circuits/nodes/relaispowernode.cpp
    circuits/nodes/relaispowernode.h
    circuits/nodes/simplecircuitnode.cpp
    circuits/nodes/simplecircuitnode.h
    circuits/nodes/simpleactivationnode.cpp
    circuits/nodes/simpleactivationnode.h

    PARENT_SCOPE
)
