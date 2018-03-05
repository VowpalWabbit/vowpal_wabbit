# test subdirectory:

configure_file("RunTests.in" "RunTests.data")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/RunTests.pl" RUNTESTS_PL)
file(READ "${CMAKE_CURRENT_BINARY_DIR}/RunTests.data" RUNTESTS_DATA)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/RunTests" "${RUNTESTS_PL}" "${RUNTESTS_DATA}")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/models")
