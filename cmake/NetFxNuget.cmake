# A collection of helpers to work with NuGet as required by VW projects.
#
# TODO: Consider switching usage to https://github.com/katusk/CMakeNuGetTools when adding CPack support.
#       Hopefully it would remove the need to specify the explicit assembly when it does not match the
#       package name, and to remove the need to manually import .props/.targets files.

find_program(netfx_nuget_COMMAND NAMES nuget REQUIRED) # TODO: Ensure this works in VW / under init.cmd

# A function to parse the NuGet Reference into consitituent parts into the specified variables
# It will export these values into its PARENT_SCOPE
#
# NuGet Reference specification:
# nuget_REFERENCE        := nuget_PACKAGE_NAME '@' nuget_PACKAGE_VERSION ['@' nuget_REFERENCE_TARGET]
# nuget_PACKAGE_NAME     := <Package name in NuGet Feed(s)>
# nuget_PACKAGE_VERSION  := <Desired version>
# nuget_REFERENCE_TARGET := <Assembly name (if different from nuget_PACKAGE_NAME)>
#                           | <.props/.targets file to import>
#
# NuGet References are used by most of the rest of the functions to specify the assembly references /
# project imports.
#
# Function arguments:
#
# package_name_var       : The target variable for the parsed nuget_PACKAGE_NAME
# package_version_var    : The target variable for the parsed nuget_PACKAGE_VERSION
# reference_target_var   : The target variable for the parsed nuget_REFERENCE_TARGET
# nuget_reference        : The nuget_REFERENCE to be parsed
# require_target         : A boolean flag specifying whether a lack of explicit REFERENCE_TARGET
#                          is a FATAL_ERROR

function(parse_nuget_reference package_name_var
                               package_version_var
                               reference_target_var
                               nuget_reference
                               require_target)

  # TODO: Change this to a single if (... MATCHES ...), to avoid semi-duplicating the RegEx
  string(REGEX REPLACE "^([^@]+)@([^@]+)(@[^@]+)?$" "\\1" nuget_PACKAGE_NAME ${nuget_reference})
  string(REGEX REPLACE "^([^@]+)@([^@]+)(@[^@]+)?$" "\\2" nuget_PACKAGE_VERSION ${nuget_reference})

  set(${package_name_var} ${nuget_PACKAGE_NAME} PARENT_SCOPE)
  set(${package_version_var} ${nuget_PACKAGE_VERSION} PARENT_SCOPE)

  if (${nuget_reference} MATCHES "^([^@]+)@([^@]+)@([^@]+)$")
    message(VERBOSE "  has import name")
    set(${reference_target_var} ${CMAKE_MATCH_3} PARENT_SCOPE)
  else()
    if (${require_target})
      message(FATAL_ERROR "Expected NuGet Reference with custom import target. Got '${nuget_reference}'.")
    endif()

    message(VERBOSE "  has no custom import name - defaulting to assembly name")
    set(${reference_target_var} ${nuget_PACKAGE_NAME} PARENT_SCOPE)
  endif()

endfunction()

# A function to install a given package, given a name and version using the found nuget command.
function(install_nuget_package package_name
                               package_version)

  message(VERBOSE "Installing ${package_name}@${package_version}")

  exec_program(${netfx_nuget_COMMAND}
    ARGS install "${package_name}" -Version ${package_version} -OutputDirectory ${CMAKE_BINARY_DIR}/packages -Verbosity Quiet)

endfunction()

function(find_nuget_lib package_name 
                        package_version)
  # find_path caches its output, so we need to treat each nuget_REFERENCE string as a unique key 
  # mapping to an assembly (the full reference is necessary in case a package contains multiple assemblies, 
  # which would require individually referencing all of them.)
  find_file(nuget_${nuget_REFERENCE}_PATH "${package_name}.${package_version}"
      HINTS "${CMAKE_BINARY_DIR}/packages"
      PATH_SUFFIXES ${netfx_nuget_SUFFIXES}
      REQUIRED
      NO_DEFAULT_PATH)
endfunction()

# (Internal) A function to add a NuGet package to a target, given a nuget_reference
function(target_add_nuget_shared target_name nuget_reference require_import)
  message(VERBOSE "For ${target_name} <== '${nuget_REFERENCE}'")

  parse_nuget_reference(nuget_PACKAGE_NAME nuget_PACKAGE_VERSION nuget_PACKAGE_IMPORT ${nuget_reference} require_import)
  message(VERBOSE "Adding NuGet package ${nuget_PACKAGE_NAME}@(${nuget_PACKAGE_VERSION}) to ${target_name}")
  message(VERBOSE "  for item ${nuget_PACKAGE_IMPORT}")

  install_nuget_package("${nuget_PACKAGE_NAME}" "${nuget_PACKAGE_VERSION}")

  # find_path caches its output, so we need to treat each nuget_REFERENCE string as a unique key
  # mapping to an assembly (the full reference is necessary in case a package contains multiple assemblies,
  # which would require individually referencing all of them.)
  find_file(nuget_${nuget_REFERENCE}_PATH "${nuget_PACKAGE_NAME}.${nuget_PACKAGE_VERSION}"
      HINTS "${CMAKE_BINARY_DIR}/packages"
      PATH_SUFFIXES ${netfx_nuget_SUFFIXES}
      REQUIRED
      NO_DEFAULT_PATH)

  message(VERBOSE "  found package at ${nuget_${nuget_REFERENCE}_PATH}")

  # TODO: CACHE?
  set(nuget_${nuget_REFERENCE}_PACKAGE_NAME ${nuget_PACKAGE_NAME} PARENT_SCOPE)
  set(nuget_${nuget_REFERENCE}_PACKAGE_VERSION ${nuget_PACKAGE_VERSION} PARENT_SCOPE)
  set(nuget_${nuget_REFERENCE}_PACKAGE_IMPORT ${nuget_PACKAGE_IMPORT} PARENT_SCOPE)
endfunction()

# A function to add NuGet assembly references to a given CMake TARGET
#
# target_name      : The CMake TARGET to update
# nuget_reference  : The nuget_REFERENCE to import to the CMake TARGET
#
# Custom nuget_REFERENCE_TARGETs are used to specify an assembly in the NuGet which does not
function(target_add_nuget_references target_name nuget_references)
  foreach(nuget_REFERENCE IN ITEMS ${nuget_references})
    target_add_nuget_shared(${target_name} ${nuget_REFERENCE} False)

    set(nuget_PACKAGE_ASSEMBLY ${nuget_${nuget_REFERENCE}_PACKAGE_IMPORT})

    message(VERBOSE "  searching for '${nuget_PACKAGE_ASSEMBLY}.dll'")
    message(VERBOSE "  searching in '${nuget_${nuget_REFERENCE}_PATH}'")

    # The ordering here might be a point of fragility - going to be good to move to DotNet Core/Net5
    # Prefer newer NetFx versions to older
    # Prefer portable to not
    # Prefer lighter profiles (e.g. non-"full") to heavier profiles
    set(netfx_nuget_SUFFIXES net46 portable-net45+win8+wpa81 net45 net45-full net40 net35 net20)

    # find_file caches its output, so we need to treat each nuget_REFERENCE string as a unique key
    # mapping to an assembly (the full reference is necessary in case a package contains multiple assemblies,
    # which would require individually referencing all of them.)
    find_file(nuget_${nuget_REFERENCE}_LIB "${nuget_PACKAGE_ASSEMBLY}.dll"
      HINTS "${nuget_${nuget_REFERENCE}_PATH}/lib"
      PATH_SUFFIXES ${netfx_nuget_SUFFIXES}
      REQUIRED
      NO_DEFAULT_PATH)

    message(VERBOSE "  found '${nuget_${nuget_REFERENCE}_LIB}'")

    set_property(TARGET ${target_name} PROPERTY
      VS_DOTNET_REFERENCE_${nuget_PACKAGE_ASSEMBLY}
      "${nuget_${nuget_REFERENCE}_LIB}")
  endforeach()
endfunction()

# A function to add NuGet assembly references to a given CMake TARGET
#
# target_name      : The CMake TARGET to update
# nuget_reference  : The nuget_REFERENCE to import to the CMake TARGET
#
# Note that for imports, the nuget_REFERENCE must have a custom nuget_REFERENCE_TARGET

function(target_add_nuget_imports target_name
                                  nuget_references)
  get_property(vs_project_imports TARGET ${target_name} PROPERTY VS_PROJECT_IMPORT)

  foreach(nuget_REFERENCE IN ITEMS ${nuget_references})
    target_add_nuget_shared(${target_name} ${nuget_REFERENCE} True)

    set(nuget_IMPORT_TARGET ${nuget_${nuget_REFERENCE}_PACKAGE_IMPORT})

    # Here "ASSEMBLY" means targets/props file, or other import
    message(VERBOSE "  searching for import '${nuget_IMPORT_TARGET}'")
    message(VERBOSE "  searching in '${nuget_${nuget_REFERENCE}_PATH}'")

    # find_file caches its output, so we need to treat each nuget_REFERENCE string as a unique key
    # mapping to an assembly (the full reference is necessary in case a package contains multiple assemblies,
    # which would require individually referencing all of them.)
    find_file(nuget_${nuget_REFERENCE}_IMPORT "${nuget_IMPORT_TARGET}"
      HINTS "${nuget_${nuget_REFERENCE}_PATH}"
      REQUIRED
      NO_DEFAULT_PATH)

    message(VERBOSE "  found '${nuget_${nuget_REFERENCE}_IMPORT}'")

    set(vs_project_imports ${vs_project_imports} "${nuget_${nuget_REFERENCE}_IMPORT}")
  endforeach()

  set_property(TARGET ${target_name} PROPERTY
    VS_PROJECT_IMPORT
    ${vs_project_imports})
endfunction()

