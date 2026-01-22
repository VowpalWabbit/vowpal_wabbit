# Python bindings using pybind11

# Generate _git_commit.py with the git commit hash
# The vw_GIT_COMMIT variable is set in the root CMakeLists.txt
set(VW_GIT_COMMIT "${vw_GIT_COMMIT}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vowpalwabbit/_git_commit.py.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/vowpalwabbit/_git_commit.py"
  @ONLY
)

if(NOT DEFINED PY_VERSION)
  set(PY_VERSION 3.10)
  message(WARNING "No PY_VERSION specified, Python ${PY_VERSION} will be used for VowpalWabbit Python bindings")
else()
  message(STATUS "Python ${PY_VERSION} will be used for VowpalWabbit Python bindings")
endif()

# Find Python - pybind11 requires both Interpreter and Development
if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  find_package(PythonInterp ${PY_VERSION} REQUIRED)
  find_package(PythonLibs ${PY_VERSION} REQUIRED)
elseif(${CMAKE_VERSION} VERSION_LESS "3.18.0")
  # Prior to 3.18 only the Development component is exposed
  find_package(Python ${PY_VERSION} EXACT COMPONENTS Interpreter Development REQUIRED)
else()
  # This was fixed in 3.18 to allow for just the Development.Module component to be used
  find_package(Python ${PY_VERSION} EXACT COMPONENTS Interpreter Development.Module REQUIRED)
endif()

# Find pybind11
# pybind11 can be installed via: pip install pybind11
# or as a system package, or as a git submodule
find_package(pybind11 REQUIRED)

if (NOT WIN32)
  # Use position independent code for all targets in this directory
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT WIN32)
  set(EXPORT_DYNAMIC -Wl,--export-dynamic)
endif()

# Create the Python extension module
pybind11_add_module(pylibvw pylibvw.cc)

# The python setup.py script should communicate what suffix it wants.
if(VW_PYTHON_SHARED_LIB_SUFFIX)
  set_target_properties(pylibvw PROPERTIES SUFFIX ${VW_PYTHON_SHARED_LIB_SUFFIX})
else()
  # fallback to previous suffix selection logic
  if (NOT WIN32)
    # OSX builds a .dylib and then attempts to find a .so. Just build a .so file and things work
    set_target_properties(pylibvw PROPERTIES SUFFIX ".so")
  else()
    # Force Windows to build with a .pyd extension
    set_target_properties(pylibvw PROPERTIES SUFFIX ".pyd")
  endif()
endif()

target_link_libraries(pylibvw PUBLIC vw_core)
target_compile_options(pylibvw PRIVATE ${EXPORT_DYNAMIC})
