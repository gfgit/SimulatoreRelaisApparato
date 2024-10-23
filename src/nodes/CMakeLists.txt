# src/nodes

add_subdirectory(edit)

set(PROJECT_SOURCES
    ${PROJECT_SOURCES}

    nodes/abstractcircuitnode.cpp
    nodes/abstractcircuitnode.h
    nodes/aceibuttonnode.cpp
    nodes/aceibuttonnode.h
    nodes/circuitcable.cpp
    nodes/circuitcable.h
    nodes/lightbulbnode.cpp
    nodes/lightbulbnode.h
    nodes/onoffswitchnode.cpp
    nodes/onoffswitchnode.h
    nodes/powersourcenode.cpp
    nodes/powersourcenode.h
    nodes/relaiscontactnode.cpp
    nodes/relaiscontactnode.h
    nodes/relaispowernode.cpp
    nodes/relaispowernode.h
    nodes/simplecircuitnode.cpp
    nodes/simplecircuitnode.h

    PARENT_SCOPE
)
