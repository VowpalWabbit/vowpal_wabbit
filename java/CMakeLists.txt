project (vw_java)

add_subdirectory(src/main/c++)

add_custom_target(java ALL
	COMMAND mvn clean test -f ${PROJECT_SOURCE_DIR}/pom.xml
	DEPENDS vw_jni)

