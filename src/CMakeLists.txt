# src

add_subdirectory(core)
add_subdirectory(enums)
add_subdirectory(graph)
add_subdirectory(nodes)
add_subdirectory(objects)
add_subdirectory(utils)

set(PROJECT_SOURCES
    ${PROJECT_SOURCES}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_sources(SimulatoreRelaisApparato
    PUBLIC
    ${PROJECT_SOURCES}
)




