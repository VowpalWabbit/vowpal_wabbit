
# Find RapidJSON header only library
#
# You can set a custom search path by using the following variable:
#     RapidJSON_CUSTOM_INCLUDEDIR - Custom search path
#
# Variables defined:
#     RapidJSON_FOUND - True if RapidJson was found
#     RapidJSON_INCLUDE_DIRS - Path to RapidJson include directory
#
# Target created:
#     RapidJSON

# If PkgConfig is found, then version of RapidJson can be found.
# Otherwise, there will be no version found.
find_package(PkgConfig)
pkg_check_modules(PkgConfig_RapidJSON QUIET RapidJSON)

find_path(RapidJSON_INCLUDE_DIR
    NAMES rapidjson.h
    PATHS ${PkgConfig_RapidJSON_INCLUDE_DIRS} ${RapidJSON_CUSTOM_INCLUDEDIR}
    PATH_SUFFIXES rapidjson
    DOC "Find the RapidJson include directory"
)

set(RapidJSON_VERSION ${PkgConfig_RapidJSON_VERSION})

mark_as_advanced(RapidJSON_FOUND RapidJSON_INCLUDE_DIR RapidJSON_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RapidJSON
    REQUIRED_VARS RapidJSON_INCLUDE_DIR
    VERSION_VAR RapidJSON_VERSION
)

if(RapidJSON_FOUND)
    set(RapidJSON_INCLUDE_DIRS ${RapidJSON_INCLUDE_DIR})
endif()

if(RapidJSON_FOUND AND NOT TARGET RapidJSON)
    add_library(RapidJSON INTERFACE IMPORTED)
    set_target_properties(RapidJSON PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RapidJSON_INCLUDE_DIR}"
    )
endif()