set(explore_all_headers
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/explore.h>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit/explore.h>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/explore_internal.h>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit/explore_internal.h>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/hash.h>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit/hash.h>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/future_compat.h>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit/future_compat.h>)

add_library(explore INTERFACE)
add_library(VowpalWabbit::explore ALIAS explore)

target_include_directories(explore INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit/>)

target_sources(explore INTERFACE ${explore_all_headers})

# Place the explore headers in their own folder in VS
source_group(explore FILES ${explore_all_headers})

if(VW_INSTALL)
  install(
    TARGETS explore
    EXPORT VowpalWabbitConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  # Explore target headers
  install(
    FILES ${explore_all_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit)
endif()
