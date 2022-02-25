add_executable(test_initialize main.cc)

# We are using a "private" header in VW and therefore must bring in the RapidJSON dependency manually.
target_link_libraries(test_initialize PRIVATE RapidJSON VowpalWabbit::vw)
