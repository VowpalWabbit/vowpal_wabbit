cmake_minimum_required (VERSION 3.5)
project (library)

set (CMAKE_CXX_STANDARD 11)

include_directories(${vw_INCLUDE_DIRS}) 

find_package(Boost COMPONENTS system thread REQUIRED)

add_executable(ezexample_predict ezexample_predict.cc)
target_link_libraries(ezexample_predict PRIVATE vw)

add_executable(ezexample_predict_threaded ezexample_predict_threaded.cc)
target_link_libraries(ezexample_predict_threaded PRIVATE vw ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(ezexample_train ezexample_train.cc)
target_link_libraries(ezexample_train PRIVATE vw)

add_executable(library_example library_example.cc)
target_link_libraries(library_example PRIVATE vw)

add_executable(test_search test_search.cc)
target_link_libraries(test_search PRIVATE vw)

add_executable(search_generate search_generate.cc)
target_link_libraries(search_generate PRIVATE vw)



if(NOT WIN32)
    add_executable(recommend recommend.cc)
    target_link_libraries(recommend PRIVATE vw)

    add_executable(gd_mf_weights gd_mf_weights.cc)
    target_link_libraries(gd_mf_weights PRIVATE vw)
endif()