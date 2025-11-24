# src/objects/traintastic

add_subdirectory(edit)

set(SIMULATORE_RELAIS_SOURCES

    objects/traintastic/traintasticsensorobj.cpp
    objects/traintastic/traintasticsensorobj.h

    objects/traintastic/traintasticturnoutobj.cpp
    objects/traintastic/traintasticturnoutobj.h

    objects/traintastic/traintasticsignalobject.cpp
    objects/traintastic/traintasticsignalobject.h

    objects/traintastic/traintasticspawnobj.cpp
    objects/traintastic/traintasticspawnobj.h

    ${SIMULATORE_RELAIS_SOURCES}
    PARENT_SCOPE
)
