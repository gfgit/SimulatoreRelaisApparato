# src/objects/simple_activable

set(SIMULATORE_RELAIS_SOURCES

    objects/simple_activable/abstractsimpleactivableobject.cpp
    objects/simple_activable/abstractsimpleactivableobject.h

    objects/simple_activable/lightbulbobject.cpp
    objects/simple_activable/lightbulbobject.h

    objects/simple_activable/electromagnet.cpp
    objects/simple_activable/electromagnet.h

    ${SIMULATORE_RELAIS_SOURCES}
    PARENT_SCOPE
)
