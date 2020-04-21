set (CMAKE_CXX_STANDARD 11)

include_directories(/usr/local/include)
find_package(flatbuffers REQUIRED)
add_executable(buffercreate write.cpp test_generated.h /usr/local/include/flatbuffers/flatbuffers.h)
add_executable(bufferread read.cpp test_generated.h /usr/local/include/flatbuffers/flatbuffers.h)
