add_executable(to_flatbuff txt_to_flat.cc vw_to_flat.cc vw_to_flat.h)
target_link_libraries(to_flatbuff PUBLIC VowpalWabbit::config VowpalWabbit::vw explore FlatbuffersTarget)