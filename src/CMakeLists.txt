# src

include(GNUInstallDirs)

include(DLL_Utils)

#Set Win32 resources
if (WIN32)
    configure_file(../packaging/windows/resources.rc.in ${CMAKE_BINARY_DIR}/resources/resources.rc)
    set(SIMULATORE_RELAIS_RESOURCES
        ${SIMULATORE_RELAIS_RESOURCES}
        ${CMAKE_BINARY_DIR}/resources/resources.rc
        )
endif()

add_subdirectory(circuits)
add_subdirectory(enums)
add_subdirectory(objects)
add_subdirectory(translations)
add_subdirectory(utils)
add_subdirectory(views)

set(SIMULATORE_RELAIS_SOURCES
  ${SIMULATORE_RELAIS_SOURCES}
  main.cpp
  mainwindow.cpp
  mainwindow.h
)

# Set SimulatoreRelaisApparato info template file
set(SIMULATORE_RELAIS_SOURCES
    ${SIMULATORE_RELAIS_SOURCES}
    info.h.in
    )
configure_file(info.h.in ${CMAKE_BINARY_DIR}/include/info.h)

# Add executable
qt_add_executable(${SIMULATORE_RELAIS_TARGET} WIN32
    MANUAL_FINALIZATION
    ${SIMULATORE_RELAIS_SOURCES}
    #${SIMULATORE_RELAIS_UI_FILES}
    ${SIMULATORE_RELAIS_RESOURCES}
)

# Set compiler options
if(MSVC)
    target_compile_options(
        ${SIMULATORE_RELAIS_TARGET}
        PRIVATE
        /WX
        /wd4267
        /wd4244
        /experimental:external
        /external:anglebrackets
        /external:W0
        "$<$<CONFIG:RELEASE>:/O2>"
        "$<$<COMPILE_LANGUAGE:CXX>:/MP>"
        )
else()
    target_compile_options(
        ${SIMULATORE_RELAIS_TARGET}
        PRIVATE
        "$<$<CONFIG:RELEASE>:-O2>"
        #-Werror
        -Wuninitialized
        -pedantic-errors
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wshadow
        )
endif()

if(UNIX AND NOT APPLE)
    target_link_options(
        ${SIMULATORE_RELAIS_TARGET}
        PRIVATE
        -rdynamic
        )
endif()

# Set include directories
target_include_directories(
    ${SIMULATORE_RELAIS_TARGET}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include #For template files
    )

# Set link libraries
target_link_libraries(
    ${SIMULATORE_RELAIS_TARGET}
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    KDAB::kddockwidgets
    )

# if (WIN32)
#     target_link_libraries(
#         ${SIMULATORE_RELAIS_TARGET}
#         PRIVATE
#         DbgHelp
#         )
# endif()

# Set compiler definitions
target_compile_definitions(${SIMULATORE_RELAIS_TARGET} PRIVATE ${SIMULATORE_RELAIS_DEFINITIONS})

## Doxygen documentation ##
if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME ${APP_DISPLAY_NAME})
    set(DOXYGEN_PROJECT_LOGO ${APP_ICON})
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
    set(DOXYGEN_EXTRACT_ALL "YES")
    set(DOXYGEN_EXTRACT_PRIVATE "YES")
    set(DOXYGEN_DOT_GRAPH_MAX_NODES 100)
    doxygen_add_docs(docs ALL ${SIMULATORE_RELAIS_SOURCES})
endif()
## Doxygen end ##

## Update/Release translations ##

#(Run this target before installing and every time you update translations)
add_custom_target(RELEASE_TRANSLATIONS ALL
    COMMENT "Running translations it_IT...")


# Run 'lupdate' to parse C++ and UI files and extract translatable strings
if(UPDATE_TS_KEEP_OBSOLETE)
    set(LUPDATE_OPTIONS_STR "")
else()
    set(LUPDATE_OPTIONS_STR "-no-obsolete")
endif()

qt_add_translations(${SIMULATORE_RELAIS_TARGET}
    TS_FILES ${SIMULATORE_RELAIS_TS_FILES}
    SOURCES ${SIMULATORE_RELAIS_SOURCES}
    LUPDATE_OPTIONS ${LUPDATE_OPTIONS_STR}
)


## Update/Release translations end ##

## Install and Deploy ##

# Copy executable
install(TARGETS ${SIMULATORE_RELAIS_TARGET})

# Copy SVG icons
# install(FILES ${CMAKE_SOURCE_DIR}/files/icons/lightning/lightning.svg
#     DESTINATION ${CMAKE_INSTALL_BINDIR}/icons)

# For each .ts file install corrensponding .qm file
foreach(TS_FILE ${SIMULATORE_RELAIS_TS_FILES})
    get_filename_component(QM_FILE_NAME ${TS_FILE} NAME_WLE)
    install(FILES
        "${CMAKE_BINARY_DIR}/src/translations/${QM_FILE_NAME}.qm"
        DESTINATION ${CMAKE_INSTALL_BINDIR}/translations OPTIONAL)
endforeach()

if(WIN32)
    # Copy KDDockWidgets-qt6 DLL
    get_dll_library_from_import_library(${KDDockWidgets-qt6_LIBRARIES} KDDockWidgets-qt6_LIBRARY_TO_INSTALL)
    install(PROGRAMS ${KDDockWidgets} DESTINATION ${CMAKE_INSTALL_BINDIR})

    if(RUN_WINDEPLOYQT)
        if(NOT TARGET windeployqt_exe)
            message(FATAL_ERROR "In order to run windeployqt you must first set the exe path in WINDEPLOYQT_EXE_DIR")
        endif()

        # Use [[...]] to delay variable expansion
        install(CODE "
            message(STATUS \"Running windeployqt ${windeployqt_exe} ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
            execute_process(COMMAND ${windeployqt_exe} ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
                WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
                OUTPUT_VARIABLE WINDEPLOYQT_EXE_RESULT
                ERROR_VARIABLE WINDEPLOYQT_EXE_RESULT)

            message(STATUS \"${WINDEPLOYQT_EXE_RESULT}\")

            message(STATUS \"${windeployqt_exe} Done.\")
            ")
    endif()
endif()

if(UNIX AND NOT APPLE)
    set(LINUX_ICON_DIR_BASE "/usr/share/icons/hicolor")

    # Install PNG icons
    set(MY_ICON_SIZES "16;24;32;64;128;256;512")
    foreach(ICON_SIZE ${MY_ICON_SIZES})
        install(FILES ${CMAKE_SOURCE_DIR}/files/icons/relais/relais_${ICON_SIZE}.png
            DESTINATION ${LINUX_ICON_DIR_BASE}/${ICON_SIZE}x${ICON_SIZE}/apps/
            RENAME ${SIMULATORE_RELAIS_TARGET}.png)

        install(FILES ${CMAKE_SOURCE_DIR}/files/icons/relais/relais_${ICON_SIZE}.png
            DESTINATION ${LINUX_ICON_DIR_BASE}/${ICON_SIZE}x${ICON_SIZE}/mimetypes/
            RENAME application-x-${SIMULATORE_RELAIS_TARGET}-json.png)
    endforeach()

    # Install SVG icon
    install(FILES ${CMAKE_SOURCE_DIR}/files/icons/relais/relais.svg
        DESTINATION ${LINUX_ICON_DIR_BASE}/scalable/apps/
        RENAME ${SIMULATORE_RELAIS_TARGET}.svg)

    install(FILES ${CMAKE_SOURCE_DIR}/files/icons/relais/relais.svg
        DESTINATION ${LINUX_ICON_DIR_BASE}/scalable/mimetypes/
        RENAME application-vnd.${APP_COMPANY_NAME_LOWER}-${SIMULATORE_RELAIS_TARGET}-json.svg)

    # Install desktop file
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/linux/${SIMULATORE_RELAIS_TARGET}.desktop
        DESTINATION /usr/share/applications/)

    # Install mime type association
    #vnd.filipposoft-simulatoreapparato.xml
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/linux/mime/vnd.${APP_COMPANY_NAME_LOWER}-${SIMULATORE_RELAIS_TARGET}-json.xml
        DESTINATION /usr/share/mime/packages)
endif(UNIX AND NOT APPLE)

## Install end ##

qt_finalize_executable(${SIMULATORE_RELAIS_TARGET})
