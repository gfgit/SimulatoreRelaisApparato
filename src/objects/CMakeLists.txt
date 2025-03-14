# src/objects

add_subdirectory(interfaces)
add_subdirectory(button)
add_subdirectory(circuit_bridge)
add_subdirectory(lever)
add_subdirectory(relais)
add_subdirectory(screen_relais)
add_subdirectory(simple_activable)

set(SIMULATORE_RELAIS_SOURCES

    objects/abstractsimulationobject.cpp
    objects/abstractsimulationobject.h

    objects/abstractsimulationobjectmodel.cpp
    objects/abstractsimulationobjectmodel.h

    objects/simulationobjectnodesmodel.cpp
    objects/simulationobjectnodesmodel.h

    objects/simulationobjectmultitypemodel.cpp
    objects/simulationobjectmultitypemodel.h

    objects/simulationobjectcopyhelper.cpp
    objects/simulationobjectcopyhelper.h

    objects/simulationobjectfactory.cpp
    objects/simulationobjectfactory.h

    objects/simulationobjectoptionswidget.cpp
    objects/simulationobjectoptionswidget.h

    objects/standardobjecttypes.cpp
    objects/standardobjecttypes.h

    objects/simulationobjectlistwidget.cpp
    objects/simulationobjectlistwidget.h

    objects/simulationobjectlineedit.cpp
    objects/simulationobjectlineedit.h

    ${SIMULATORE_RELAIS_SOURCES}
    PARENT_SCOPE
)
