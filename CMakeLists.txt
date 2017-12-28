cmake_minimum_required (VERSION 3.5)

# Common configuration

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
set(THREADS_PREFER_PTHREAD_FLAG ON)
#set(OPENSSL_USE_STATIC_LIBS TRUE)

set(VW_COMMON_SOURCE common/vw_exception.cc)

# NOTE: The order matters! The most independent ones should go first.
add_subdirectory(common)
add_subdirectory(allreduce)
add_subdirectory(vowpalwabbit) 
add_subdirectory(library) 
add_subdirectory(decision_service) 

# add_subdirectory(components/Executable) # Executable (depends on A and C)


# install(TARGETS ${PROJECT_NAME}
        # RUNTIME DESTINATION bin
        # LIBRARY DESTINATION lib
        # ARCHIVE DESTINATION lib/static
        # PUBLIC_HEADER DESTINATION include)
