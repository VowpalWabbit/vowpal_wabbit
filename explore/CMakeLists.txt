set(explore_all_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/explore.h
  ${CMAKE_CURRENT_SOURCE_DIR}/explore_internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/hash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/future_compat.h)

add_library(explore INTERFACE)
target_include_directories(explore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(explore INTERFACE ${explore_all_headers})

install(
  TARGETS explore
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)

install(
  FILES ${explore_all_headers}
  DESTINATION include/vowpalwabbit/vowpalwabbit/explore)

# Place the explore headers in their own folder in VS
source_group(explore FILES ${explore_all_headers})
