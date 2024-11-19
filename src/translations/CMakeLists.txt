# src/translations

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM TRUE)

set(SIMULATORE_RELAIS_TS_FILES
  translations/simra_it.ts
)

set_source_files_properties(src/translations/simra_it.ts
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

set(SIMULATORE_RELAIS_TS_FILES
  ${SIMULATORE_RELAIS_TS_FILES}
  PARENT_SCOPE
)
