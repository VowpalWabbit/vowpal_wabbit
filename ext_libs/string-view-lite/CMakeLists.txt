add_library(string_view INTERFACE)

add_library(VowpalWabbit::string_view ALIAS string_view)

target_include_directories(string_view INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(VW_INSTALL)
  install(
    TARGETS string_view
    EXPORT VowpalWabbitConfig)

  install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/vowpalwabbit
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit
  )
endif()
