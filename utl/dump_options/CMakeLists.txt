add_executable(vw-dump-options main.cc)
target_link_libraries(vw-dump-options PUBLIC VowpalWabbit::vw RapidJSON)
