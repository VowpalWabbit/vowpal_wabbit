find_package(JNI)

if(JNI_FOUND)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(src_base src/main/c++)
  set(vw_spark_jni_headers
    ${src_base}/jni_spark_vw.h
  )

  set(vw_spark_jni_sources
    ${src_base}/jni_spark_vw.cc
  )

  add_library(vw_spark_jni SHARED ${vw_spark_jni_headers} ${vw_spark_jni_sources})
  target_link_libraries(vw_spark_jni PUBLIC vw)
  target_include_directories(vw_spark_jni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${JNI_INCLUDE_DIRS})

  # Ensure target directory exists
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target/)
  add_custom_command(TARGET vw_spark_jni POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vw_spark_jni> ${CMAKE_CURRENT_SOURCE_DIR}/target/
  )

  # Replace version number in POM
  configure_file(pom.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/pom.xml @ONLY)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(JAVA_INSTALL_PATH /usr/lib)
  else()
      set(JAVA_INSTALL_PATH /Library/Java/Extensions)
  endif()

  if(VW_INSTALL)
    install(TARGETS vw_spark_jni
      RUNTIME DESTINATION ${JAVA_INSTALL_PATH}
      LIBRARY DESTINATION ${JAVA_INSTALL_PATH}
    )
  endif()
endif()
