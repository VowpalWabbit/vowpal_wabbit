ifeq ($(JAVA_HOME),)
  $(warning No JAVA_HOME found, JNI building will fail.  Please set JAVA_HOME when using JNI)
  exit 1
endif

VWLIBS := -L../vowpalwabbit -l vw
STDLIBS = $(BOOST_LIBRARY) $(LIBS)
JAVA_INCLUDE = -I $(JAVA_HOME)/include
LIB_NAME = "vw_jni"

ifeq ($(UNAME), Linux)
  OS_DEPENDENT_LIB_TARGET = "lib$(LIB_NAME).so"
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
endif
ifeq ($(UNAME), FreeBSD)
  OS_DEPENDENT_LIB_TARGET = "lib$(LIB_NAME).so"
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  OS_DEPENDENT_LIB_TARGET = "$(LIB_NAME).dll"
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/linux
endif
ifeq ($(UNAME), Darwin)
  OS_DEPENDENT_LIB_TARGET = "lib$(LIB_NAME).dylib"
  JAVA_INCLUDE += -I $(JAVA_HOME)/include/darwin
endif

jni_SRCS = $(shell find src/main/c++ -name "*.cc")
jni_OBJS = $(jni_SRCS:.cc=.o)

all:
	cd ..; $(MAKE)

test:
	cd ..; $(MAKE) test

things: pom_version target/$(OS_DEPENDENT_LIB_TARGET)

pom_version: pom.xml
	ver=$$(grep AC_INIT ../configure.ac | cut -d '[' -f 3 | cut -d ']' -f 1) && \
	newVer=$$(perl -e "@a=split('\.', '$$ver'); \$$a[2]++; print(join('.', @a))") && \
	perl -pi -e "s/(\s*)<version>.*-SNAPSHOT/\1<version>$$newVer-SNAPSHOT/" pom.xml

target/$(OS_DEPENDENT_LIB_TARGET): $(jni_OBJS) ../vowpalwabbit/main.o ../vowpalwabbit/libvw.a ../vowpalwabbit/liballreduce.a
	mkdir -p target;
	$(CXX) -shared $(FLAGS) -o $@ $^ $(VWLIBS) $(STDLIBS) $(JAVA_INCLUDE)

%.o: %.cc  %.h
	$(CXX) $(FLAGS) -c $< -o $@ $(JAVA_INCLUDE)

%.o: %.cc
	$(CXX) $(FLAGS) -c $< -o $@ $(JAVA_INCLUDE)

	-include $(jni_SRCS:.cc=.o)

.PHONY: clean
clean:
	rm -f target/$(OS_DEPENDANT_LIB_TARGET)
	rm -f $(jni_OBJS) 
