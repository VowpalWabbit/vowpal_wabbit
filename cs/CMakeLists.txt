add_subdirectory(common) 
# this only works in x64
# still needs further investigation on how to link against VowpalWabbit.Common
# if(${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
# add_subdirectory(cli) 
# add_subdirectory(cs) 
# endif()
