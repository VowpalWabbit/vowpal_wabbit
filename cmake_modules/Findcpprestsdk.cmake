# FindCasablanca package
#
# Tries to find the Casablanca (C++ REST SDK) library
#


# Include dir
find_path(cpprest_INCLUDE_DIR
  NAMES
    cpprest/http_client.h
  PATHS 
    ${cpprest_PKGCONF_INCLUDE_DIRS}
    ${cpprest_DIR}
    $ENV{cpprest_DIR}
    /usr/local/include
    /usr/include
    ../../casablanca
  PATH_SUFFIXES 
    Release/include
    include
)

# Library
IF(UNIX)
find_package(PkgConfig)

	include(LibFindMacros)

	find_library(cpprest_LIBRARY
  	NAMES 
    	cpprest
  	PATHS 
    	${cpprest_PKGCONF_LIBRARY_DIRS}
    	${cpprest_DIR}
    	${cpprest_DIR}
    	$ENV{cpprest_DIR}
    	/usr/local
    	/usr
    	../../casablanca
  	PATH_SUFFIXES
    	lib
    	Release/build.release/Binaries/
    	build.release/Binaries/
	)
	
	set(cpprest_PROCESS_LIBS cpprest_LIBRARY)
	set(cpprest_PROCESS_INCLUDES cpprest_INCLUDE_DIR)
	libfind_process(cpprest)
ELSE()
	find_library(cpprest_LIBRARY NAMES cpprest_2_10)
ENDIF()
