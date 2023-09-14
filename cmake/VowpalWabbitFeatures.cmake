# We control library features with the feature list, including conditional
# compilation or optional parts.
#
# This feature list can be specified by:
#   - Specifying individual features with -DVW_FEATURE_FEAT1=ON/OFF
#
# This allows us to enable/disable features without changing the code
# If a feature called X is enabled then:
#   - The cmake variable VW_FEAT_X is set to ON, otherwise it is OFF
#   - The C++ macro VW_FEAT_X_ENABLED is defined if the feature is enabled, otherwise it is not defined

set(VW_ALL_FEATURES "CSV;FLATBUFFERS;LDA;CB_GRAPH_FEEDBACK;SEARCH;LAS_SIMD;NETWORKING")

option(VW_FEAT_FLATBUFFERS "Enable flatbuffers support" OFF)
option(VW_FEAT_CSV "Enable csv parser" ON)
option(VW_FEAT_CB_GRAPH_FEEDBACK "Enable cb with graph feedback reduction" OFF)
option(VW_FEAT_LDA "Enable lda reduction" ON)
option(VW_FEAT_SEARCH "Enable search reductions" ON)
option(VW_FEAT_LAS_SIMD "Enable large action space with explicit simd (only works with linux for now)" ON)
option(VW_FEAT_NETWORKING "Enable daemon mode, spanning tree, sender, and active" ON)

# Legacy options for feature enablement
if(DEFINED BUILD_FLATBUFFERS)
  message(WARNING "BUILD_FLATBUFFERS is deprecated. Use -DVW_FEAT_FLATBUFFERS=On instead.")
  set(VW_FEAT_FLATBUFFERS ${BUILD_FLATBUFFERS} CACHE BOOL "" FORCE)
endif()

if(DEFINED VW_BUILD_CSV)
  message(WARNING "VW_BUILD_CSV is deprecated. Use -DVW_FEAT_CSV=On instead.")
  set(VW_FEAT_CSV ${VW_BUILD_CSV} CACHE BOOL "" FORCE)
endif()

if(DEFINED VW_BUILD_LAS_WITH_SIMD)
  message(WARNING "VW_BUILD_LAS_WITH_SIMD is deprecated. Use -DVW_FEAT_LAS_SIMD=On instead.")
  set(VW_FEAT_LAS_SIMD ${VW_BUILD_LAS_WITH_SIMD} CACHE BOOL "" FORCE)
endif()

function(vw_print_enabled_features)
  foreach(feature ${VW_ALL_FEATURES})
    if(VW_FEAT_${feature})
      list(APPEND ALL_ENABLED_FEATURES ${feature})
    endif()
  endforeach()
  set(VW_ALL_ENABLED_FEATURES "${ALL_ENABLED_FEATURES}" PARENT_SCOPE)

  # Print enabled features
  message(STATUS "Enabled features: ${ALL_ENABLED_FEATURES} (Available features: ${VW_ALL_FEATURES})")
endfunction()
