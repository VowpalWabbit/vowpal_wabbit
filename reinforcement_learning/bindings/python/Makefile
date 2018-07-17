# Underscore is required to be consistent with Swig's name generation
TARGET = _rlinference.so

# Python 3.6 is specifically provided, tricky if we want to link against different versions.
INCLUDE = -I ../../include -I ./ $(shell python3.6-config --includes)
RLCLIENT_LIB = -L ../../rlclientlib -lrlclient
VW_LIB = -L ../../../vowpalwabbit -lvw -lallreduce
BOOST_LIBS = -lboost_system
CPPREST_LIBS = -lcpprest -lssl -lcrypto -pthread -ldl
ALL_LIBS = $(RLCLIENT_LIB) $(VW_LIB) $(BOOST_LIBS) $(CPPREST_LIBS) $(LIBS)

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

MODULE_NAME = rlinference
MAIN_SWIG_INTERFACE = $(MODULE_NAME).i
OTHER_SWIG_INTERFACES = $(filter-out $(MAIN_SWIG_INTERFACE), $(wildcard *.i))
MAIN_SWIG_WRAPPED_INTERFACE = $(MODULE_NAME)_wrap.cc

# $(MAIN_SWIG_WRAPPED_INTERFACE) may or may not be present when this is calculated, in order to only add it once we filter it out here.
SOURCES = $(filter-out $(MAIN_SWIG_WRAPPED_INTERFACE), $(wildcard *.cc))
SOURCES += $(MAIN_SWIG_WRAPPED_INTERFACE)
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(shell find $(CURDIR)/../../include . -name '*.h')

$(MAIN_SWIG_WRAPPED_INTERFACE): $(MAIN_SWIG_INTERFACE) $(OTHER_SWIG_INTERFACES)
	swig -c++ -python -o $@ $<

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(FLAGS) -shared -o $@ $(OBJECTS) $(LIBDIR) $(ALL_LIBS)

test:
	python3.6 -m unittest -v test

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f $(MODULE_NAME)_wrap.cc
	-rm -f $(MODULE_NAME)_wrap.h
	-rm -f $(MODULE_NAME).py
	-rm -f $(MODULE_NAME).pyc
