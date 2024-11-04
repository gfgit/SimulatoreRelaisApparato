# src

add_subdirectory(circuits)
add_subdirectory(enums)
add_subdirectory(objects)
add_subdirectory(utils)
add_subdirectory(views)

set(PROJECT_SOURCES
    ${PROJECT_SOURCES}
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_sources(SimulatoreRelaisApparato
    PUBLIC
    ${PROJECT_SOURCES}
)




