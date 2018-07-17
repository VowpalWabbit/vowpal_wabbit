# Underscore is required to be consistent with Swig's name generation
TARGET = _rlinference.so

# Python 3.6 is specifically provided, tricky if we want to link against different versions.
INCLUDE = -I ../include -I ./ $(shell python3.6-config --includes)

RLCLIENT_LIB = -L ../rlclientlib -lrlclient
ALL_LIBS = $(RLCLIENT_LIB)

FLAGS += -fPIC -std=c++11

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = $(shell find . -name '*.cc' ! -name '*_wrap.cc')
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(shell find $(CURDIR)/../include . -name '*.h')

MODULE_NAME = rlinference
MAIN_SWIG_INTERFACE = $(MODULE_NAME).i
OTHER_SWIG_INTERFACES = $(filter-out $(MAIN_SWIG_INTERFACE), $(wildcard *.i))
MAIN_SWIG_WRAPPED_INTERFACE = $(MODULE_NAME)_wrap.cc
SOURCES += $(MAIN_SWIG_WRAPPED_INTERFACE)
$(MAIN_SWIG_WRAPPED_INTERFACE): $(MAIN_SWIG_INTERFACE) $(OTHER_SWIG_INTERFACES)
	swig -c++ -python -o $@ $<

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS) $(WRAPPED_INTERFACES)

rl_clientlib:
	cd ../rlclientlib; $(MAKE)

$(TARGET): rl_clientlib $(OBJECTS)
	$(CXX) $(FLAGS) -shared -o $@ $(OBJECTS) $(LIBDIR) $(ALL_LIBS)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f $(MODULE_NAME)_wrap.cc
	-rm -f $(MODULE_NAME)_wrap.h
	-rm -f $(MODULE_NAME).py
	-rm -f $(MODULE_NAME).pyc
