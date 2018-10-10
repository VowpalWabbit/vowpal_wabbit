add_subdirectory(unit_test)

# Don't offer target on Windows
if(NOT WIN32)
  add_custom_target(test
    DEPENDS all_library_examples unit_test
    COMMAND ./RunTests -d -fe -E 0.001 -O --onethread ../vowpalwabbit/vw
    COMMAND ./RunTests -d -fe -E 0.001 ../vowpalwabbit/vw
    COMMAND python save_resume_test.py --verbose_on_fail
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
