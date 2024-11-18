# src/objects

add_subdirectory(interfaces)
add_subdirectory(lever)
add_subdirectory(relais)
add_subdirectory(simple_activable)

set(PROJECT_SOURCES

    objects/abstractsimulationobject.cpp
    objects/abstractsimulationobject.h

    objects/abstractsimulationobjectmodel.cpp
    objects/abstractsimulationobjectmodel.h

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

    ${PROJECT_SOURCES}
    PARENT_SCOPE
)
