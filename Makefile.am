SUBDIRS = vowpalwabbit cluster library

if PYTHON_MODULE
SUBDIRS += python
endif
if JAVA_JNI_MODULE
SUBDIRS += java
endif

noinst_HEADERS = \
	explore/static/utility.h \
	explore/static/MWTExplorer.h

ACLOCAL_AMFLAGS = -I acinclude.d

test: all
	cd test && ./RunTests -f -E 0.001 ../vowpalwabbit/vw ../vowpalwabbit/vw
