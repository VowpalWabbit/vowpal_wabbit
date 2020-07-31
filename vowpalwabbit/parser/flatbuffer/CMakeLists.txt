set(EXAMPLE_SCHEMA
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/example.fbs")

build_flatbuffers("${EXAMPLE_SCHEMA}" "" fbschemas "" "${CMAKE_CURRENT_SOURCE_DIR}/generated" "" "")

