# We control library features with the feature list, including conditional
# compilation or optional parts.
#
# This feature list can be specified by:
#   - Specifying individual features with -DVW_FEATURE_FEAT1_ENABLED=ON/OFF
#
# This allows us to enable/disable features without changing the code
# If a feature called X is enabled then:
#   - The cmake variable VW_FEATURE_X_ENABLED is set to ON, otherwise it is OFF
#   - The C++ macro VW_FEATURE_X_ENABLED is defined if the feature is enabled, otherwise it is not defined
#
# Features:
#   FLATBUFFERS: Enable flatbuffers support
#   CSV: Enable csv parser
#   LAS_SIMD: Enable large action space with explicit simd (only work with linux for now)

set(VW_ALL_FEATURES "LAS_SIMD;FLATBUFFERS;CSV")

option(VW_FEATURE_FLATBUFFERS_ENABLED "Enable flatbuffers support" OFF)
option(VW_FEATURE_CSV_ENABLED "Enable csv parser" OFF)
option(VW_FEATURE_LAS_SIMD_ENABLED "Enable large action space with explicit simd (only works with linux for now)" ON)

# Legacy options for feature enablement
if(DEFINED BUILD_FLATBUFFERS)
  message(WARNING "BUILD_FLATBUFFERS is deprecated. Use -DVW_FEATURE_FLATBUFFERS_ENABLED=On instead.")
  set(VW_FEATURE_FLATBUFFERS_ENABLED ${BUILD_FLATBUFFERS} CACHE BOOL "")
endif()

if(DEFINED VW_BUILD_CSV)
  message(WARNING "VW_BUILD_CSV is deprecated. Use -DVW_FEATURE_CSV_ENABLED=On instead.")
  set(VW_FEATURE_CSV_ENABLED ${VW_BUILD_CSV} CACHE BOOL "")
endif()

if(DEFINED VW_BUILD_LAS_WITH_SIMD)
  message(WARNING "VW_BUILD_LAS_WITH_SIMD is deprecated. Use -DVW_FEATURE_LAS_SIMD_ENABLED=On instead.")
  set(VW_FEATURE_LAS_SIMD_ENABLED ${VW_BUILD_LAS_WITH_SIMD} CACHE BOOL "")
endif()

function(vw_print_enabled_features)
  foreach(feature ${VW_ALL_FEATURES})
    if(VW_FEATURE_${feature}_ENABLED)
      list(APPEND ALL_ENABLED_FEATURES ${feature})
    endif()
  endforeach()

  # Print enabled features
  message(STATUS "Enabled features: ${ALL_ENABLED_FEATURES} (Available features: ${VW_ALL_FEATURES})")
endfunction()
