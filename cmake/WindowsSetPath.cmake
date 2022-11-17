
# This function will add shared libraries to the PATH when running the test, so
# they can be found. Windows does not support RPATH or similar. See:
# https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order
# Usage: windows_set_path(<test> <target>...)
function(windows_set_path TEST)
  if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    return()
  endif()

  set(targets_path "")
  set(glue "")
  foreach(target IN LISTS ARGN)
    if(TARGET "${target}")
      get_target_property(type "${target}" TYPE)
      if(type STREQUAL "SHARED_LIBRARY")
        set(targets_path "${targets_path}${glue}$<TARGET_FILE_DIR:${target}>")
        set(glue "\;") # backslash is important
      endif()
    endif()
  endforeach()
  if(NOT targets_path STREQUAL "")
    set_property(TEST "${TEST}" PROPERTY ENVIRONMENT "PATH=$ENV{PATH};${targets_path}")
  endif()
endfunction()