project (vw_java)

find_package(Java)

if (Java_FOUND)
	add_subdirectory(src/main/c++)

	configure_file(pom.xml.in ${PROJECT_SOURCE_DIR}/pom.xml @ONLY)

	add_custom_target(java ALL
		COMMAND mvn -q clean test -f pom.xml
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		DEPENDS vw_jni)
endif()
