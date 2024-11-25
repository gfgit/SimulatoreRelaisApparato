# src/objects/interfaces/mechanical

#add_subdirectory(model)
#add_subdirectory(view)

set(SIMULATORE_RELAIS_SOURCES

    objects/interfaces/mechanical/mechanicalcondition.cpp
    objects/interfaces/mechanical/mechanicalcondition.h

    ${SIMULATORE_RELAIS_SOURCES}
    PARENT_SCOPE
)
