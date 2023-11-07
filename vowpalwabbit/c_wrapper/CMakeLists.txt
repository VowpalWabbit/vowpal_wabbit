set(lib_type "SHARED_ONLY")
if(STATIC_LINK_VW)
  set(lib_type "STATIC_OR_SHARED")
endif()

vw_add_library(
    NAME "c_wrapper"
    TYPE ${lib_type}
    SOURCES include/vw/c_wrapper/vwdll.h include/vw/c_wrapper/vwvis.h src/vwdll.cc
    PRIVATE_DEPS vw_core
    DESCRIPTION "Old C API. Cannot convey errors in current interface. Can be disabled using option `VW_BUILD_VW_C_WRAPPER`"
    EXCEPTION_DESCRIPTION "Yes, exceptions are thrown across boundary"
    ENABLE_INSTALL
)

target_compile_definitions(vw_c_wrapper PUBLIC VWDLL_EXPORTS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE ZLIB_WINAPI)

vw_add_test_executable(
  FOR_LIB "c_wrapper"
  SOURCES tests/vwdll_test.cc
  EXTRA_DEPS vw_core
)

# Also for compat install to old location.
if(VW_INSTALL)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/include/vw/c_wrapper/vwdll.h ${CMAKE_CURRENT_BINARY_DIR}/vwdll.h)
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/vwdll.h "#pragma warning \"This header location is deprecated and will be removed in a future release. Please use vw/c_wrapper/vwdll.h\"\n")
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/vwdll.h
      ${CMAKE_CURRENT_LIST_DIR}/include/vw/c_wrapper/vwvis.h
    DESTINATION
      ${CMAKE_INSTALL_INCLUDEDIR}/vowpalwabbit/
  )
endif()
