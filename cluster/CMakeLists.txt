set(vowpal_wabbit_dir "${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit")

add_executable(spanning_tree
spanning_tree_main.cc
${vowpal_wabbit_dir}/spanning_tree.cc
${vowpal_wabbit_dir}/vw_exception.cc)

# Winsock32 should be available on Windows
if(WIN32)
  target_link_libraries(spanning_tree PRIVATE wsock32 ws2_32)
endif()

if(STATIC_LINK_VW)
  target_link_libraries(spanning_tree PRIVATE ${unix_static_flag})
endif()

target_include_directories(spanning_tree PRIVATE ${vowpal_wabbit_dir})
target_link_libraries(spanning_tree PRIVATE ${LINK_THREADS})


if(VW_INSTALL)
  install(
    TARGETS spanning_tree
    EXPORT VowpalWabbitConfig
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
