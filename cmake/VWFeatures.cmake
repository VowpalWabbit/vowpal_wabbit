# We control library features with the feature list, including conditional
# compilation or optional parts.
#
# This feature list can be specified in two ways:
#   - Specifying -DVW_ENABLED_FEATURES="FEAT1;FEAT2"
#   - Specifying individual features with -DVW_FEATURE_FEAT1_ENABLED=ON/OFF
#
# Note: If both are specified, the individual feature enabled flags take precedence.
#
# This allows us to enable/disable features without changing the code
# The list of features is defined as a cmake list (; separated)
# If a feature called X is enabled then:
#   - The cmake variable VW_FEATURE_X_ENABLED is se to ON, otherwise it is OFF
#   - The C++ macro VW_FEATURE_X_ENABLED is defined if the feature is enabled, otherwise it is not defined
#
# If a feature has conditions for its enablement it can be disabled with:
#   vw_disable_feature(FEATURE_NAME REASON)
#
# This MUST be done prior to include(VowpalWabbitUtils), and after include(VWFeatures)
#
# Features:
#   FLATBUFFERS: Enable flatbuffers support
#   CSV: Enable csv parser
#   LAS_SIMD: Enable large action space with explicit simd (only work with linux for now)

set(VW_DEFAULT_FEATURES "LAS_SIMD")
set(VW_ALL_FEATURES "LAS_SIMD;FLATBUFFERS;CSV")

set(VW_ENABLED_FEATURES "${VW_DEFAULT_FEATURES}" CACHE STRING "List of features to enable.")

# Legacy options for feature enablement
option(BUILD_FLATBUFFERS "Build flatbuffers" OFF)
if(BUILD_FLATBUFFERS)
  message(WARNING "BUILD_FLATBUFFERS is deprecated. Use -DVW_ENABLED_FEATURES=other_features;flatbuffers instead.")
  list(APPEND VW_ENABLED_FEATURES "FLATBUFFERS")
endif()

option(VW_BUILD_CSV "Build csv parser" OFF)
if(VW_BUILD_CSV)
  message(WARNING "VW_BUILD_CSV is deprecated. Use -DVW_ENABLED_FEATURES=other_features;csv instead.")
  list(APPEND VW_ENABLED_FEATURES "CSV")
endif()

option(VW_BUILD_LAS_WITH_SIMD "Build large action space with explicit simd (only work with linux for now)" OFF)
if(VW_BUILD_LAS_WITH_SIMD)
  message(WARNING "VW_BUILD_LAS_WITH_SIMD is deprecated. Use -DVW_ENABLED_FEATURES=other_features;las_simd instead.")
  list(APPEND VW_ENABLED_FEATURES "LAS_SIMD")
endif()

# Check if there are any duplicates in the list
set(VW_ENABLED_FEATURES_COPY ${VW_ENABLED_FEATURES})
list(LENGTH VW_ENABLED_FEATURES_COPY VW_ENABLED_FEATURES_LENGTH)
list(REMOVE_DUPLICATES VW_ENABLED_FEATURES_COPY)
list(LENGTH VW_ENABLED_FEATURES_COPY VW_ENABLED_FEATURES_LENGTH_NO_DUPLICATES)
if(NOT VW_ENABLED_FEATURES_LENGTH EQUAL VW_ENABLED_FEATURES_LENGTH_NO_DUPLICATES)
  message(FATAL_ERROR "Duplicate features specified in VW_ENABLED_FEATURES. Values: ${VW_ENABLED_FEATURES}")
endif()

# Iterate list of features and check if they exist
foreach(feature ${VW_ENABLED_FEATURES})
  string(TOUPPER ${feature} feature_upper)
  # Check if exists in all features
  if(NOT "${feature_upper}" IN_LIST VW_ALL_FEATURES)
    message(FATAL_ERROR "Feature ${feature_upper} is not a valid feature. Valid features are: ${VW_ALL_FEATURES}")
  endif()
endforeach()

# Convert VW_ENABLED_FEATURES to uppercase
foreach(feature ${VW_ENABLED_FEATURES})
  string(TOUPPER ${feature} feature_upper)
  list(APPEND VW_ENABLED_FEATURES_UPPER ${feature_upper})
endforeach()

set(VW_FEATURE_COMPILE_DEFINITIONS "")

# Iterate all features and enable ones which were passed:
foreach(feature ${VW_ALL_FEATURES})
  string(TOUPPER ${feature} feature_upper)

  # Check if was explicitly passed and evaluates to false, then ignore the list
  if(DEFINED VW_FEATURE_${feature_upper}_ENABLED AND NOT VW_FEATURE_${feature_upper}_ENABLED)
    continue()
  endif()

  if("${feature_upper}" IN_LIST VW_ENABLED_FEATURES_UPPER)
    set(VW_FEATURE_${feature_upper}_ENABLED ON)
  else()
    set(VW_FEATURE_${feature_upper}_ENABLED OFF)
  endif()
endforeach()

# Create VW_FEATURE_COMPILE_DEFINITIONS list
foreach(feature ${VW_ALL_FEATURES})
  string(TOUPPER ${feature} feature_upper)
  if(VW_FEATURE_${feature_upper}_ENABLED)
    list(APPEND VW_FEATURE_COMPILE_DEFINITIONS "VW_FEATURE_${feature_upper}_ENABLED")
    list(APPEND FINAL_FEATURES ${feature_upper})
  endif()
endforeach()

# Print enabled features
message(STATUS "Enabled features: ${FINAL_FEATURES} (Available features: ${VW_ALL_FEATURES})")

unset(FINAL_FEATURES)
unset(VW_DEFAULT_FEATURES)
unset(VW_ALL_FEATURES)
unset(VW_ENABLED_FEATURES_UPPER)
unset(VW_ENABLED_FEATURES)

function(vw_disable_feature FEATURE_NAME REASON)
  string(TOUPPER ${FEATURE_NAME} feature_upper)
  if (NOT DEFINED VW_FEATURE_${feature_upper}_ENABLED)
    message(FATAL_ERROR "Feature ${feature_upper} is not a valid feature.")
  endif()
  if (NOT VW_FEATURE_${feature_upper}_ENABLED)
    message(FATAL_ERROR "Feature ${feature_upper} is already disabled.")
  endif()
  message(STATUS "Disabling feature ${feature_upper}: ${REASON}")
  set(VW_FEATURE_${feature_upper}_ENABLED OFF CACHE BOOL "Disable ${feature_upper} feature" FORCE)
  set(${OUTPUT} vw_${LIB_NAME} PARENT_SCOPE)
  list(REMOVE_ITEM VW_FEATURE_COMPILE_DEFINITIONS "VW_FEATURE_${feature_upper}_ENABLED")
  set(VW_FEATURE_COMPILE_DEFINITIONS ${VW_FEATURE_COMPILE_DEFINITIONS} PARENT_SCOPE)
endfunction()

# Outputs:
#   VW_FEATURE_COMPILE_DEFINITIONS: List of compile definitions for enabled features
#   VW_FEATURE_<FEATURE>_ENABLED: CMake variable for each feature
