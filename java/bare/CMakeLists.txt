if(NOT WIN32)

find_package(Java)
find_package(JNI)
include(UseJava)

if(JNI_FOUND)

  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(src_base ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c++)
  set(vw_bare_jni_headers
    ${src_base}/jni_bare_vw.h
    ${src_base}/jni_bare_vw_generated.h
    ${src_base}/vector_io_buf.h
    ${src_base}/util.h
  )
  set(vw_bare_jni_sources
    ${src_base}/jni_bare_vw.cc
    ${src_base}/vector_io_buf.cc
    ${src_base}/jni_bare_cluster.cc
    ${src_base}/util.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vowpalwabbit/spanning_tree.cc)

  # Ensure target directory exists
  file(MAKE_DIRECTORY target/classes)
  file(MAKE_DIRECTORY target/bin/natives/linux_64)

  # required for jni header generation
  # add_custom_target(javacompile 
    # COMMAND mvn compile
    # WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # COMMENT "Compile classes for javah")

  # useful to generate the jni headers
  # create_javah(TARGET javaheaders
  #   CLASSES
  #     org.vowpalwabbit.bare.VowpalWabbitNative
  #     org.vowpalwabbit.bare.VowpalWabbitExample
  #     org.vowpalwabbit.bare.ClusterSpanningTree
  #   CLASSPATH ${CMAKE_CURRENT_SOURCE_DIR}/target/classes
  #   OUTPUT_NAME ${CMAKE_CURRENT_SOURCE_DIR}/src/main/c++/jni_bare_vw_generated.h)
  # add_dependencies(javaheaders javacompile)

  add_library(vw_bare_jni SHARED ${vw_bare_jni_headers} ${vw_bare_jni_sources})
  # add_dependencies(vw_bare_jni javaheaders)

  target_compile_options(vw_bare_jni PUBLIC ${linux_flags})

  # enable-new-dtags and rpath enables shared object library lookup in the location of libvw_bare_jni.so
  target_link_libraries(vw_bare_jni PUBLIC -Wl,--enable-new-dtags -Wl,-rpath,\"\$ORIGIN\" vw)

  target_include_directories(vw_bare_jni PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JNI_INCLUDE_DIRS} 
    ${explore_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vowpalwabbit)

  # TODO: support Win/osx 64/32/ ..

  # that doesn't work with ldd below
  # set_target_properties(vw_bare_jni PROPERTIES LIBRARY_OUTPUT_DIRECTORY target/bin/natives/linux_64)

  add_custom_command(TARGET vw_bare_jni POST_BUILD
    COMMAND ldd $<TARGET_FILE:vw_bare_jni> | grep -E 'boost|libz' | grep -oP '=> \\K\\S+' | xargs -i cp {} target/bin/natives/linux_64
    # COMMAND mkdir target/bin/natives/linux_64
    COMMAND cp $<TARGET_FILE:vw_bare_jni> target/bin/natives/linux_64
    COMMAND echo $<TARGET_FILE:vw-bin> > ${CMAKE_CURRENT_SOURCE_DIR}/target/test-classes/vw-bin.txt
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
    install(TARGETS vw_bare_jni
      RUNTIME DESTINATION ${JAVA_INSTALL_PATH}
      LIBRARY DESTINATION ${JAVA_INSTALL_PATH}
    )
  endif()
endif()

endif()