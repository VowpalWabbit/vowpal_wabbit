VWLIBS := -L../vowpalwabbit -l vw
STDLIBS = $(BOOST_LIBRARY) $(LIBS)
JAVA_INCLUDE = -I $(JAVA_HOME)/include
LIBRARY_NAME = vw_jni

ifeq ($(UNAME), Linux)
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
  SHARED_LIBRARY = lib$(LIBRARY_NAME).so
  INSTALL_PATH = /usr/lib
endif
ifeq ($(UNAME), FreeBSD)
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
  SHARED_LIBRARY = lib$(LIBRARY_NAME).so
  INSTALL_PATH = /usr/lib
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
  SHARED_LIBRARY = lib$(LIBRARY_NAME).so
  INSTALL_PATH = /usr/lib
endif
ifeq ($(UNAME), Darwin)
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/darwin
  SHARED_LIBRARY = lib$(LIBRARY_NAME).dylib
  INSTALL_PATH = /Library/Java/Extensions
endif

LOCAL_LIBRARY = target/$(SHARED_LIBRARY)
jni_SRCS = $(shell find src/main/c++ -name "*.cc")
jni_OBJS = $(jni_SRCS:.cc=.o)

all:
	cd ..; $(MAKE)

test:
	cd ..; $(MAKE) test

things: pom_version $(LOCAL_LIBRARY)

pom_version: pom.xml
	ver=$$(grep AC_INIT ../configure.ac | cut -d '[' -f 3 | cut -d ']' -f 1) && \
	newVer=$$(perl -e "@a=split('\.', '$$ver'); \$$a[2]++; print(join('.', @a))") && \
	perl -pi -e "s/(\s*)<version>.*-SNAPSHOT/\1<version>$$newVer-SNAPSHOT/" pom.xml

$(LOCAL_LIBRARY): $(jni_OBJS) ../vowpalwabbit/main.o ../vowpalwabbit/libvw.a ../vowpalwabbit/liballreduce.a
	mkdir -p target;
	$(CXX) -shared $(FLAGS) -o $@ $^ $(VWLIBS) $(STDLIBS) $(JAVA_INCLUDE)

%.o: %.cc  %.h
	$(CXX) $(FLAGS) -c $< -o $@ $(JAVA_INCLUDE)

%.o: %.cc
	$(CXX) $(FLAGS) -c $< -o $@ $(JAVA_INCLUDE)

	-include $(jni_SRCS:.cc=.o)

install: $(LOCAL_LIBRARY)
	cp $(LOCAL_LIBRARY) $(INSTALL_PATH);

.PHONY: clean
clean:
	rm -f $(LOCAL_LIBRARY)
	rm -f $(jni_SRCS:.cc=.o)
