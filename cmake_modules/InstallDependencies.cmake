macro (Install_Dependencies BINARY)

IF (WIN32)
	# help to find the dlls on windows
	IF (cpprest_LIB_DIR)
		get_filename_component(cpprest_LIB_DIR ${cpprest_LIBRARY} DIRECTORY)
	ENDIF()

	IF (Boost_LIBRARY_DIRS)
	   list(GET Boost_LIBRARY_DIRS 0 Boost_LIBRARY_DIRS_0)
	   SET(Boost_BIN_DIRS "${Boost_LIBRARY_DIRS_0}/../bin")
	ENDIF()
	message("Boost bin dirs: ${Boost_BIN_DIRS}" )

	IF (openssl_LIB_DIR)
		get_filename_component(openssl_LIB_DIR ${OPENSSL_SSL_LIBRARY} DIRECTORY)
	ENDIF()
	
	# ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/
	INSTALL(CODE "
		INCLUDE(GetPrerequisites)
		GET_PREREQUISITES(${BINARY} DEPENDENCIES 1 1 \"\"
				\"${Boost_BIN_DIRS}\"
				\"${cpprest_LIB_DIR}../bin\"
				\"${openssl_LIB_DIR}../bin\")

		FOREACH(DEPENDENCY \${DEPENDENCIES})
		   GET_FILENAME_COMPONENT(DEPENDENCY_NAME \"\${DEPENDENCY}\" NAME)

		   find_file (DEPENDENCY_FINAL \"\${DEPENDENCY_NAME}\"
				HINTS
				\"${Boost_BIN_DIRS}\"
				\"${cpprest_LIB_DIR}../bin\"
				\"${openssl_LIB_DIR}../bin\")
		   message(\"Dependency \${DEPENDENCY} -> \${DEPENDENCY_FINAL}\")
		
		   FILE(INSTALL \"\${DEPENDENCY_FINAL}\" DESTINATION \"${CMAKE_INSTALL_PREFIX}/bin\")

		   unset(DEPENDENCY_FINAL CACHE)
		ENDFOREACH()
	")
ENDIF()

endmacro()
