# Underscore is required to be consistent with Swig's name generation
TARGET = _rlinference.so

# Python 3.6 is specifically provided, tricky if we want to link against different versions.
INCLUDE = -I ../include -I ../rlclientlib -I ../../explore -I ../explore -I ../../rapidjson/include -I ./ $(shell python3.6-config --includes)

VW_LIB = -L ../../vowpalwabbit -lvw -lallreduce
BOOST_LIBS = -lboost_system -lboost_program_options
CPPREST_LIBS = -lcpprest -lssl -lcrypto -pthread -ldl
ALL_LIBS = $(VW_LIB) $(BOOST_LIBS) $(CPPREST_LIBS) $(LIBS)

FLAGS += -fPIC -std=c++11

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = $(shell find $(CURDIR)/../rlclientlib . -name '*.cc' ! -name '*_wrap.cc')
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(shell find $(CURDIR)/../rlclientlib $(CURDIR)/../include . -name '*.h')

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

$(TARGET): $(OBJECTS)
	$(CXX) $(FLAGS) -shared -o $@ $(OBJECTS) $(LIBDIR) $(ALL_LIBS)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f ../rlclientlib/*.o
	-rm -f $(MODULE_NAME)_wrap.cc
	-rm -f $(MODULE_NAME)_wrap.h
	-rm -f $(MODULE_NAME).py
	-rm -f $(MODULE_NAME).pyc
