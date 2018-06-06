project (doc)
find_package(Doxygen)

if (DOXYGEN_FOUND)
	add_custom_target(
		doc_doxygen ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile)
endif()
