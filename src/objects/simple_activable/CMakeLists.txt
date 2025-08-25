# src/objects/simple_activable

set(SIMULATORE_RELAIS_SOURCES

    objects/simple_activable/abstractsimpleactivableobject.cpp
    objects/simple_activable/abstractsimpleactivableobject.h

    objects/simple_activable/lightbulbobject.cpp
    objects/simple_activable/lightbulbobject.h

    objects/simple_activable/electromagnetobject.cpp
    objects/simple_activable/electromagnetobject.h

    objects/simple_activable/soundobject.cpp
    objects/simple_activable/soundobject.h

    objects/simple_activable/abstractactivableobjectsmodel.cpp
    objects/simple_activable/abstractactivableobjectsmodel.h

    ${SIMULATORE_RELAIS_SOURCES}
    PARENT_SCOPE
)
