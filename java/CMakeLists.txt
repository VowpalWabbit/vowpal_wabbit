project (vw_java)

find_package(Java)

if (Java_FOUND)
	add_subdirectory(src/main/c++)

	configure_file(pom.xml.in ${PROJECT_SOURCE_DIR}/pom.xml @ONLY)

	# TODO: should this run by default?
	add_custom_target(java # ALL
		COMMAND mvn compile -f pom.xml
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		DEPENDS vw_jni)

	add_custom_target(java_test
		COMMAND mvn test -f pom.xml
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		DEPENDS vw_jni)		
endif()