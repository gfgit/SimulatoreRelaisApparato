# src/circuits

add_subdirectory(edit)
add_subdirectory(graphs)
add_subdirectory(nodes)
add_subdirectory(view)

set(SIMULATORE_RELAIS_SOURCES
    ${SIMULATORE_RELAIS_SOURCES}

    circuits/circuitscene.cpp
    circuits/circuitscene.h

    circuits/electriccircuit.cpp
    circuits/electriccircuit.h

    PARENT_SCOPE
)
