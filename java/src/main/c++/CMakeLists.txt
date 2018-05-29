project (vw_jni)

set(CMAKE_CXX_STANDARD 11)

find_package(JNI REQUIRED)
include_directories(${vw_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED jni_base_learner.cc vowpalWabbit_learner_VWMulticlassLearner.cc vowpalWabbit_learner_VWScalarsLearner.cc vowpalWabbit_learner_VWActionProbsLearner.cc vowpalWabbit_learner_VWMultilabelsLearner.cc vowpalWabbit_VW.cc vowpalWabbit_learner_VWActionScoresLearner.cc vowpalWabbit_learner_VWProbLearner.cc vowpalWabbit_learner_VWLearners.cc vowpalWabbit_learner_VWScalarLearner.cc)

set_target_properties(${PROJECT_NAME} PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/java/target)


target_link_libraries(${PROJECT_NAME} PRIVATE vw)
