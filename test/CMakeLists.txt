project (vw_test)
enable_testing()

find_package(Perl 5.14)

if(${PERL_FOUND})
	add_test(NAME RunTestsSingleThread
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMAND ${PERL_EXECUTABLE} RunTests -d -fe -E 0.001 -O --onethread $<TARGET_FILE:vw-bin>)
	add_test(NAME RunTestsDefault
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMAND ${PERL_EXECUTABLE} RunTests -d -fe -E 0.001 $<TARGET_FILE:vw-bin>)


	if(MSVC)
	add_custom_target(RunTests
		COMMAND set "PATH=%PATH%;${cpprest_LIBRARY}/../../bin"
		COMMAND ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
			  DEPENDS vw-bin)
	else()
	add_custom_target(RunTests
		COMMAND ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
			  DEPENDS vw-bin)
	endif()
endif()

find_package(PythonInterp)

if(${PYTHONINTERP_FOUND} AND ${PYTHON_VERSION_MAJOR} EQUAL "3")
	# save_resume_test.py needs python 3
	add_test(NAME RunSaveResumeTest
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMAND ${PYTHON_EXECUTABLE} save_resume_test.py --verbose_on_fail --vw $<TARGET_FILE:vw-bin>)
endif()