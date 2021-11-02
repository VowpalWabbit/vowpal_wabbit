add_subdirectory(src)

if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif ()
