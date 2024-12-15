# src/panels

add_subdirectory(edit)
add_subdirectory(graphs)
add_subdirectory(view)

set(SIMULATORE_RELAIS_SOURCES
    ${SIMULATORE_RELAIS_SOURCES}

    panels/abstractpanelitem.cpp
    panels/abstractpanelitem.h

    panels/panelscene.cpp
    panels/panelscene.h

    PARENT_SCOPE
)
