vw_add_executable(
  NAME "spanning_tree"
  OVERRIDE_BIN_NAME "spanning_tree"
  SOURCES "src/spanning_tree_main.cc"
  DEPS vw_spanning_tree vw_common vw_config
  ENABLE_INSTALL
)

add_executable(spanning_tree ALIAS vw_spanning_tree_bin)

if(STATIC_LINK_VW)
  target_link_libraries(vw_spanning_tree_bin PRIVATE ${unix_static_flag})
endif()
