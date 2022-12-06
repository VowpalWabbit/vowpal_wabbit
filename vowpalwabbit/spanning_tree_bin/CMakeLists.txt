vw_add_executable(
  NAME "spanning_tree"
  OVERRIDE_BIN_NAME "spanning_tree"
  SOURCES "src/spanning_tree_main.cc"
  DEPS vw_spanning_tree vw_common vw_config vw_io
  DESCRIPTION "Command line tool for connecting instances of vw for distributed learning"
  ENABLE_INSTALL
)

if(STATIC_LINK_VW)
  target_link_libraries(vw_spanning_tree_bin PRIVATE ${unix_static_flag})
endif()
