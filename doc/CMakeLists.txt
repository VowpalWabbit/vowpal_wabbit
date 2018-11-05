find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
  message(WARNING "Doxygen needs to be installed to generate the doxygen documentation")
else()
  set(DOXYGEN_SKIP_DOT TRUE)

  # TODO properly do an out of source doxygen build - https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
