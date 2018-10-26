find_package(JNI)

if(JNI_FOUND)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(src_base src/main/c++)
  set(vw_jni_headers
    ${src_base}/jni_base_learner.h
    ${src_base}/vowpalWabbit_learner_VWActionProbsLearner.h
    ${src_base}/vowpalWabbit_learner_VWActionScoresLearner.h
    ${src_base}/vowpalWabbit_learner_VWLearners.h
    ${src_base}/vowpalWabbit_learner_VWMulticlassLearner.h
    ${src_base}/vowpalWabbit_learner_VWMultilabelsLearner.h
    ${src_base}/vowpalWabbit_learner_VWProbLearner.h
    ${src_base}/vowpalWabbit_learner_VWScalarLearner.h
    ${src_base}/vowpalWabbit_learner_VWScalarsLearner.h
    ${src_base}/vowpalWabbit_VW.h
  )

  set(vw_jni_sources
    ${src_base}/jni_base_learner.cc
    ${src_base}/vowpalWabbit_learner_VWActionProbsLearner.cc
    ${src_base}/vowpalWabbit_learner_VWActionScoresLearner.cc
    ${src_base}/vowpalWabbit_learner_VWLearners.cc
    ${src_base}/vowpalWabbit_learner_VWMulticlassLearner.cc
    ${src_base}/vowpalWabbit_learner_VWMultilabelsLearner.cc
    ${src_base}/vowpalWabbit_learner_VWProbLearner.cc
    ${src_base}/vowpalWabbit_learner_VWScalarLearner.cc
    ${src_base}/vowpalWabbit_learner_VWScalarsLearner.cc
    ${src_base}/vowpalWabbit_VW.cc
  )

  add_library(vw_jni SHARED ${vw_jni_headers} ${vw_jni_sources})
  target_link_libraries(vw_jni PUBLIC vw)
  target_include_directories(vw_jni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${JNI_INCLUDE_DIRS})

  add_custom_command(TARGET vw_jni POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vw_jni> ${CMAKE_CURRENT_SOURCE_DIR}/target/
  )

  # Replace version number in POM
  configure_file(pom.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/pom.xml @ONLY)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(JAVA_INSTALL_PATH /usr/lib)
  else()
      set(JAVA_INSTALL_PATH /Library/Java/Extensions)
  endif()

  install(TARGETS vw_jni
    RUNTIME DESTINATION ${JAVA_INSTALL_PATH}
    LIBRARY DESTINATION ${JAVA_INSTALL_PATH}
  )
endif()
