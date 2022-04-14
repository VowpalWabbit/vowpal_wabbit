set(lib_type "SHARED_ONLY")
if(STATIC_LINK_VW)
  set(lib_type "STATIC_OR_SHARED")
endif()

vw_add_library(
    NAME "c_wrapper"
    TYPE ${lib_type}
    SOURCES include/vw/c_wrapper/vwdll.h src/vwdll.cc
    PUBLIC_DEPS vw_common
    PRIVATE_DEPS vw_core
    ENABLE_INSTALL
)
target_compile_definitions(vw_c_wrapper PUBLIC VWDLL_EXPORTS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE ZLIB_WINAPI)

include(WindowsSetPath)

vw_add_test_executable(
  FOR_LIB "c_wrapper"
  SOURCES tests/vwdll_test.cc
  EXTRA_DEPS vw_core
)
windows_set_path(vw_c_wrapper_test vw_c_wrapper)
