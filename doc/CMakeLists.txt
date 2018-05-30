project (doc)
find_package(Doxygen)

# if (Doxygen::doxygen)
message("markus")
add_custom_target(
	doc_doxygen ALL
	# Doxygen::doxygen
	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile)
	# doxygen_add_docs(${project_name} Doxyfile)
# endif()
