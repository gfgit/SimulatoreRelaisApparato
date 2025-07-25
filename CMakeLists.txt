cmake_minimum_required(VERSION 3.16)

project(SimulatoreRelaisApparato VERSION 0.8.2 LANGUAGES CXX)

option(UPDATE_TS_KEEP_OBSOLETE "Keep obsolete entries when updating translations" ON)
option(BUILD_DOXYGEN "Build Doxygen documentation" OFF)

if (WIN32)
    option(RUN_WINDEPLOYQT "Run windeployqt after executable is installed" ON)
endif()

## Defines ##

set(APP_PRODUCT_NAME "SimulatoreRelais")
set(APP_DISPLAY_NAME "Simulatore Relais Apparato")
set(APP_COMPANY_NAME "FiliPPoSoft")
set(APP_COMPANY_NAME_LOWER "filipposoft")

set(PROJECT_HOMEPAGE_URL "https://github.com/gfgit/SimulatoreRelaisApparato")
set(APP_HELP_URL ${PROJECT_HOMEPAGE_URL})
set(APP_UPDATE_URL ${PROJECT_HOMEPAGE_URL})

string(CONCAT PROJECT_DESCRIPTION "${APP_DISPLAY_NAME} is a cross-platform C++\n"
                        "application with Qt GUI for simulating relais of Italian railway interlockings.")

set(PROJECT_DESCRIPTION_SHORT "Simulator for relais of Italian railways interlockings")

set(APP_ICON ${CMAKE_SOURCE_DIR}/files/icons/relais/relais.ico)

# Create main application target
set(SIMULATORE_RELAIS_TARGET "simulatoreapparato")

## defines end ##

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Include our custom FindXXX moudules in '/cmake' subdirectory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Qt6 REQUIRED
    COMPONENTS
    Core
    Gui
    Widgets
    Network
    SerialPort
    Svg
    Multimedia
    LinguistTools)

find_package(KDDockWidgets-qt6 REQUIRED)

if(BUILD_DOXYGEN)
    find_package(Doxygen)
endif()

if (WIN32)
    include(LocateWinDeployQt)
endif()


add_subdirectory(src)
add_subdirectory(packaging)

include(Packing)
