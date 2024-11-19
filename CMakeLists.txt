cmake_minimum_required(VERSION 3.16)

project(SimulatoreRelaisApparato VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(KDDockWidgets-qt6 REQUIRED)


qt_add_executable(SimulatoreRelaisApparato
    MANUAL_FINALIZATION
)


if (WIN32)
    # Fix KDAB::kddockwidgets include path
    # It is originally set to "${_IMPORT_PREFIX}/include/kddockwidgets-qt6"
    # But to be compatible with Linux we want just "${_IMPORT_PREFIX}/include"

    get_target_property(kddockwidgets_incl_old KDAB::kddockwidgets INTERFACE_INCLUDE_DIRECTORIES)
    cmake_path(GET kddockwidgets_incl_old PARENT_PATH kddockwidgets_incl_fix)

    target_include_directories(SimulatoreRelaisApparato PRIVATE ${kddockwidgets_incl_fix})
    message("Fixed KDAB::kddockwidgets include path: ${kddockwidgets_incl_fix}")
endif (WIN32)


add_subdirectory(src)

target_link_libraries(SimulatoreRelaisApparato
    PRIVATE
    Qt6::Widgets
    KDAB::kddockwidgets)

set_target_properties(SimulatoreRelaisApparato PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SimulatoreRelaisApparato
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(SimulatoreRelaisApparato)
