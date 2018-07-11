project (doc_proj)
find_package(Doxygen)

if (DOXYGEN_FOUND)
	add_custom_target(
		# TODO: want that this is added to all?
		doc # ALL 
		COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile)
endif()
