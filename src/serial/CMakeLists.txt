# src/serial

add_subdirectory(view)

set(SIMULATORE_RELAIS_SOURCES
    ${SIMULATORE_RELAIS_SOURCES}

    serial/serialdevice.cpp
    serial/serialdevice.h

    serial/serialmanager.cpp
    serial/serialmanager.h

    serial/serialdevicesmodel.cpp
    serial/serialdevicesmodel.h

    PARENT_SCOPE
)
