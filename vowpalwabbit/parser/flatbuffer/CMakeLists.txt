
find_package(flatbuffers REQUIRED)

set(EXAMPLE_SCHEMA
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/example.fbs")

build_flatbuffers("${EXAMPLE_SCHEMA}" "" fbschemas "" "${CMAKE_CURRENT_SOURCE_DIR}/generated" "" "")

#add_executable(buffercreate write.cpp test_generated.h /usr/local/include/flatbuffers/flatbuffers.h)
#add_executable(buf_with_preample write_with_preamble.cc test_generated.h /usr/local/include/flatbuffers/flatbuffers.h preamble.h preamble.cc endian.h endian.cc)
#add_executable(bufferread read.cpp test_generated.h /usr/local/include/flatbuffers/flatbuffers.h)
#add_executable(buf_read_preample read_with_preamble.cc test_generated.h /usr/local/include/flatbuffers/flatbuffers.h preamble.h preamble.cc endian.h endian.cc)

