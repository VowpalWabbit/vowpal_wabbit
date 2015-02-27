pyexec_LTLIBRARIES = pylibvw.la

things: all-am

pylibvw_la_SOURCES = pylibvw.cc
pylibvw_la_CXXFLAGS = $(BOOST_CPPFLAGS) $(PYTHON_CPPFLAGS) $(PTHREAD_CFLAGS)
pylibvw_la_LDFLAGS = -module -shared -avoid-version \
	$(BOOST_LDFLAGS) $(PYTHON_LDFLAGS) \
	$(BOOST_PYTHON_LIB) \
	$(PYTHON_EXTRA_LIBS)
pylibvw_la_LIBADD = ../vowpalwabbit/libvw.la ../vowpalwabbit/liballreduce.la