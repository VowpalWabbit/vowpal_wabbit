AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = vw_jni.la

vw_jni_la_CXXFLAGS = $(CXXFLAGS) $(CXXFLAGS_JNI_INCLUDES) ${BOOST_CPPFLAGS}
vw_jni_la_SOURCES = src/main/c++/vw_VWScorer.cc ../vowpalwabbit/main.cc
vw_jni_la_LIBADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la
vw_jni_la_LDFLAGS = -module -shared -avoid-version \
	$(BOOST_LDFLAGS) \
	$(BOOST_PYTHON_LIB)

noinst_HEADERS = \
	src/main/c++/vw_VWScorer.h
