vw_add_executable(
    NAME "cli"
    OVERRIDE_BIN_NAME "vw"
    SOURCES "src/main.cc"
    DEPS vw_core
    DESCRIPTION "Primary VW command line interface. The `vw` executable."

    ENABLE_INSTALL
)

if(NOT WIN32)
  find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)
  if (HELP2MAN_EXECUTABLE)
    add_custom_target(manpage ALL
      COMMAND ${HELP2MAN_EXECUTABLE}
        --no-info
        --name="Vowpal Wabbit -- fast online learning tool" $<TARGET_FILE:vw_cli_bin>
        --output=$<TARGET_FILE:vw_cli_bin>.1
      DEPENDS vw_cli_bin)

      if(VW_INSTALL)
        install(FILES $<TARGET_FILE:vw_cli_bin>.1 DESTINATION share/man/man1)
      endif()
  else()
    message(STATUS "help2man not found, please install it to generate manpages")
  endif()
endif()

if(STATIC_LINK_VW)
  target_link_libraries(vw_cli_bin PRIVATE ${unix_static_flag})
endif()