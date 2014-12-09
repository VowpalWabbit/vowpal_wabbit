ifeq ($(JAVA_HOME),)
  $(warning No JAVA_HOME found, JNI building will fail.  Please set JAVA_HOME when using JNI)
  exit 1
endif

VWLIBS := -L../vowpalwabbit -l vw -l allreduce
STDLIBS = $(BOOST_LIBRARY) $(LIBS)
JAVA_INCLUDE = -I $(JAVA_HOME)/include

ifeq ($(UNAME), Linux)
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
endif
ifeq ($(UNAME), FreeBSD)
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
endif
ifeq ($(UNAME), Darwin)
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/darwin
endif

all:
	cd ..; $(MAKE)

test:
	cd ..; $(MAKE) test
	
things: target/vw_jni.lib	

target/vw_jni.lib: src/main/c++/vw_VWScorer.cc ../vowpalwabbit/main.o ../vowpalwabbit/libvw.a ../vowpalwabbit/liballreduce.a
	mkdir -p target;
	$(CXX) -shared $(FLAGS) -o $@ $< $(VWLIBS) $(STDLIBS) $(JAVA_INCLUDE)

clean:
	rm -f target/vw_jni.lib