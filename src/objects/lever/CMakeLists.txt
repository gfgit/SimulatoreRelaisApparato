# src/objects/lever

add_subdirectory(model)
add_subdirectory(view)

add_subdirectory(acei)
add_subdirectory(ace_sasib)


set(SIMULATORE_RELAIS_SOURCES
    ${SIMULATORE_RELAIS_SOURCES}
    PARENT_SCOPE
)
