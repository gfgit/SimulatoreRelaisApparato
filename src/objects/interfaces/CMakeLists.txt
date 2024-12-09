# src/objects/interfaces

add_subdirectory(mechanical)

set(SIMULATORE_RELAIS_SOURCES

    objects/interfaces/abstractobjectinterface.cpp
    objects/interfaces/abstractobjectinterface.h

    objects/interfaces/bemhandleinterface.cpp
    objects/interfaces/bemhandleinterface.h

    objects/interfaces/buttoninterface.cpp
    objects/interfaces/buttoninterface.h

    objects/interfaces/leverinterface.cpp
    objects/interfaces/leverinterface.h

    objects/interfaces/mechanicalinterface.cpp
    objects/interfaces/mechanicalinterface.h

    objects/interfaces/sasibaceleverextrainterface.cpp
    objects/interfaces/sasibaceleverextrainterface.h

    ${SIMULATORE_RELAIS_SOURCES}
    PARENT_SCOPE
)
