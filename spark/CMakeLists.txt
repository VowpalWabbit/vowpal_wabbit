find_package(Java)
find_package(JNI)
include(UseJava)

# set(Boost_USE_STATIC_LIBS ON)
# SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# SET(BUILD_SHARED_LIBS OFF)

if(JNI_FOUND)

# target_include_directories(vw PUBLIC ${explore_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
# target_compile_definitions(vw PUBLIC _FILE_OFFSET_BITS=64 $<$<CONFIG:RELEASE>:NDEBUG>)


  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(src_base ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c++)
  set(vw_spark_jni_headers
    ${src_base}/jni_spark_vw.h
    ${src_base}/jni_spark_vw_generated.h
    ${src_base}/vector_io_buf.h
    ${src_base}/util.h
  )
  set(vw_spark_jni_sources
    ${src_base}/jni_spark_vw.cc
    ${src_base}/vector_io_buf.cc
    ${src_base}/jni_spark_cluster.cc
    ${src_base}/util.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit/spanning_tree.cc)

  create_javah(TARGET javaheaders
    CLASSES
      vowpalwabbit.spark.VowpalWabbitNative
      vowpalwabbit.spark.VowpalWabbitExample
      vowpalwabbit.spark.ClusterSpanningTree
    CLASSPATH ${CMAKE_CURRENT_SOURCE_DIR}/target/classes
    OUTPUT_NAME ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c++/jni_spark_vw_generated.h)

  add_library(vw_spark_jni SHARED ${vw_spark_jni_headers} ${vw_spark_jni_sources})
  target_include_directories(vw_spark_jni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit)
  target_compile_options(vw_spark_jni PUBLIC ${linux_flags})
  # enable-new-dtags and rpath enables shared object library lookup in the location of libvw_spark_jni.so
  target_link_libraries(vw_spark_jni PUBLIC -Wl,--enable-new-dtags -Wl,-rpath,\"\$ORIGIN\" vw)
  target_include_directories(vw_spark_jni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${JNI_INCLUDE_DIRS} ${explore_INCLUDE_DIRS})

  # Ensure target directory exists
  file(MAKE_DIRECTORY target/bin/natives/linux_64)
  # TODO: support Win/osx 64/32/ ..

  # that doesn't work with ldd below
  # set_target_properties(vw_spark_jni PROPERTIES LIBRARY_OUTPUT_DIRECTORY target/bin/natives/linux_64)

  add_custom_command(TARGET vw_spark_jni PRE_BUILD
	COMMAND mvn compile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Compile classes for javah")
	
  add_custom_command(TARGET vw_spark_jni POST_BUILD
	COMMAND ldd $<TARGET_FILE:vw_spark_jni> | grep -E 'boost|libz' | grep -oP '=> \\K\\S+' | xargs -i cp {} target/bin/natives/linux_64
	COMMAND cp $<TARGET_FILE:vw_spark_jni> target/bin/natives/linux_64
	COMMAND mvn verify
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Copying shared libary dependencies to output directory")

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
