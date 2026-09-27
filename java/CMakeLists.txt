# 3.11+ is required to use add_jar
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# make sure it works with OpenJDK
set(JAVA_AWT_LIBRARY NotNeeded)
set(JAVA_JVM_LIBRARY NotNeeded)

find_package(JNI REQUIRED)
find_package(Java)
include(UseJava)

# This jar is just to make use of the GENERATE_NATIVE_HEADERS functionality.
# The vw_jni_generate_native_headers_do_not_use_jar.jar itself should not be used in any way.
add_jar(vw_jni_generate_native_headers_do_not_use_jar
SOURCES
  src/main/java/common/Native.java
  src/main/java/common/NativePlatform.java
  src/main/java/org/vowpalwabbit/spark/ClusterSpanningTree.java
  src/main/java/org/vowpalwabbit/spark/prediction
  src/main/java/org/vowpalwabbit/spark/prediction/ScalarPrediction.java
  src/main/java/org/vowpalwabbit/spark/VowpalWabbitArguments.java
  src/main/java/org/vowpalwabbit/spark/VowpalWabbitExample.java
  src/main/java/org/vowpalwabbit/spark/VowpalWabbitMurmur.java
  src/main/java/org/vowpalwabbit/spark/VowpalWabbitNative.java
  src/main/java/org/vowpalwabbit/spark/VowpalWabbitPerformanceStatistics.java
  src/main/java/vowpalWabbit/learner/package-info.java
  src/main/java/vowpalWabbit/learner/VWActionProbsLearner.java
  src/main/java/vowpalWabbit/learner/VWActionScoresLearner.java
  src/main/java/vowpalWabbit/learner/VWBase.java
  src/main/java/vowpalWabbit/learner/VWCCBLearner.java
  src/main/java/vowpalWabbit/learner/VWFloatLearner.java
  src/main/java/vowpalWabbit/learner/VWIntLearner.java
  src/main/java/vowpalWabbit/learner/VWLearner.java
  src/main/java/vowpalWabbit/learner/VWLearnerBase.java
  src/main/java/vowpalWabbit/learner/VWLearners.java
  src/main/java/vowpalWabbit/learner/VWMulticlassLearner.java
  src/main/java/vowpalWabbit/learner/VWMultilabelsLearner.java
  src/main/java/vowpalWabbit/learner/VWProbLearner.java
  src/main/java/vowpalWabbit/learner/VWScalarLearner.java
  src/main/java/vowpalWabbit/learner/VWScalarsLearner.java
  src/main/java/vowpalWabbit/learner/VWTypedLearner.java
  src/main/java/vowpalWabbit/learner/VWNoPredLearner.java
  src/main/java/vowpalWabbit/learner/VWActionPDFValueLearner.java
  src/main/java/vowpalWabbit/learner/VWPDFLearner.java
  src/main/java/vowpalWabbit/learner/VWActiveMulticlassLearner.java
  src/main/java/vowpalWabbit/responses/ActionProb.java
  src/main/java/vowpalWabbit/responses/ActionProbs.java
  src/main/java/vowpalWabbit/responses/ActionScore.java
  src/main/java/vowpalWabbit/responses/ActionScores.java
  src/main/java/vowpalWabbit/responses/DecisionScores.java
  src/main/java/vowpalWabbit/responses/ActiveMulticlass.java
  src/main/java/vowpalWabbit/responses/Multilabels.java
  src/main/java/vowpalWabbit/responses/PDF.java
  src/main/java/vowpalWabbit/responses/PDFSegment.java
  src/main/java/vowpalWabbit/responses/PDFValue.java
  src/main/java/vowpalWabbit/VW.java
GENERATE_NATIVE_HEADERS vw_jni_native_headers)

set(src_base src/main/c++)
set(vw_jni_headers
  ${src_base}/jni_base_learner.h
  ${src_base}/vowpalWabbit_VW.h
  ${src_base}/jni_spark_vw.h
  ${src_base}/util.h
)

set(vw_jni_sources
  ${src_base}/jni_base_learner.cc
  ${src_base}/vowpalWabbit_learner_VWCCBLearner.cc
  ${src_base}/vowpalWabbit_learner_VWActionProbsLearner.cc
  ${src_base}/vowpalWabbit_learner_VWActionScoresLearner.cc
  ${src_base}/vowpalWabbit_learner_VWLearners.cc
  ${src_base}/vowpalWabbit_learner_VWMulticlassLearner.cc
  ${src_base}/vowpalWabbit_learner_VWMultilabelsLearner.cc
  ${src_base}/vowpalWabbit_learner_VWProbLearner.cc
  ${src_base}/vowpalWabbit_learner_VWScalarLearner.cc
  ${src_base}/vowpalWabbit_learner_VWScalarsLearner.cc
  ${src_base}/vowpalWabbit_learner_VWNoPredLearner.cc
  ${src_base}/vowpalWabbit_learner_VWActionPDFValueLearner.cc
  ${src_base}/vowpalWabbit_learner_VWPDFLearner.cc
  ${src_base}/vowpalWabbit_learner_VWActiveMulticlassLearner.cc
  ${src_base}/vowpalWabbit_VW.cc
  ${src_base}/jni_spark_vw.cc
  ${src_base}/jni_spark_cluster.cc
  ${src_base}/util.cc
)

add_library(vw_jni SHARED ${vw_jni_headers} ${vw_jni_sources})
target_link_libraries(vw_jni
  PUBLIC vw_core vw_spanning_tree
  PRIVATE vw_jni_native_headers
)
set_property(TARGET vw_jni PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET vw_jni PROPERTY BUILD_RPATH_USE_ORIGIN ON)
target_include_directories(vw_jni PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${JNI_INCLUDE_DIRS})

# Determine platform-specific natives directory.
#
# These names are the contract with common.Native.platformDir(), which recomputes them at
# runtime on the consumer's machine. Nothing but java/src/test/java/common/
# NativePlatformTest.java connects the two, so change both together. They drifted once --
# windows_64 here against windows_x64 there -- and the Windows native in the published
# 9.11.1 JAR was unloadable while every build and test passed.
#
# Lowercased first because CMAKE_SYSTEM_PROCESSOR is "ARM64" on Windows and "arm64" or
# "aarch64" elsewhere.
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" VW_JNI_PROCESSOR)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(VW_JNI_PROCESSOR MATCHES "aarch64|arm64")
    set(VW_JNI_NATIVES_DIR "linux_arm64")
  else()
    set(VW_JNI_NATIVES_DIR "linux_64")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if(VW_JNI_PROCESSOR MATCHES "aarch64|arm64")
    set(VW_JNI_NATIVES_DIR "macos_arm64")
  else()
    set(VW_JNI_NATIVES_DIR "macos_x64")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # This branch used to ignore the processor, so a Windows ARM64 build would have landed
  # in windows_64 while the loader looked in windows_arm64.
  if(VW_JNI_PROCESSOR MATCHES "aarch64|arm64")
    set(VW_JNI_NATIVES_DIR "windows_arm64")
  else()
    set(VW_JNI_NATIVES_DIR "windows_64")
  endif()
else()
  # Previously a warning that set the directory to "unknown". The build then succeeded, the
  # JAR assembled with a native in it, `jar tf` found one, and no loader could ever locate
  # it. BUILD_JAVA is opt-in, so if it was asked for on a platform we cannot name, saying
  # so is more useful than shipping an unloadable artifact.
  message(FATAL_ERROR
    "Cannot determine the Java natives directory for ${CMAKE_SYSTEM_NAME} "
    "(${CMAKE_SYSTEM_PROCESSOR}). Add it here and to common.Native.platformDir(), or "
    "configure without -DBUILD_JAVA=On.")
endif()
message(STATUS "VW Java natives directory: ${VW_JNI_NATIVES_DIR}")

# Ensure target directory exists
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target/bin/natives/${VW_JNI_NATIVES_DIR}/)
add_custom_command(TARGET vw_jni POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vw_jni> ${CMAKE_CURRENT_SOURCE_DIR}/target/bin/natives/${VW_JNI_NATIVES_DIR}/
)

# Enable new dtags for RPATH on Linux only (ELF-specific feature)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(vw_jni PRIVATE "-Wl,--enable-new-dtags")
endif()

# Replace version number in POM
configure_file(pom.xml.in ${CMAKE_CURRENT_SOURCE_DIR}/pom.xml @ONLY)

# Write vw_cli_bin path for integration tests (using file(GENERATE) for cross-platform support)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target/test-classes)
file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/target/test-classes/vw_cli_bin.txt
    CONTENT "$<TARGET_FILE:vw_cli_bin>\n")

if(NOT WIN32)
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(JAVA_INSTALL_PATH /Library/Java/Extensions)
  else()
    set(JAVA_INSTALL_PATH /usr/lib)
  endif()

  if(VW_INSTALL)
      install(TARGETS vw_jni
      RUNTIME DESTINATION ${JAVA_INSTALL_PATH}
      LIBRARY DESTINATION ${JAVA_INSTALL_PATH}
      )
  endif()
endif()
