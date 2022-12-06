
add_library(string_view_lite INTERFACE)
target_include_directories(string_view_lite SYSTEM INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(nonstd::string-view-lite ALIAS string_view_lite)

if(VW_INSTALL)
  install(
    TARGETS string_view_lite
    EXPORT VowpalWabbitConfig)

  install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/nonstd/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nonstd/
  )
endif()
