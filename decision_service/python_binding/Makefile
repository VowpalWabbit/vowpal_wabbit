# Underscore is required to be consistent with Swig's name generation
TARGET = _rlinference.so

# Python 3.6 is specifically provided, tricky if we want to link against different versions.
INCLUDE = -I ../include -I ../rlclientlib -I ../../explore -I ../explore -I ../../rapidjson/include -I ./ -I /usr/include/python3.6m

VW_LIB = -L ../../vowpalwabbit -lvw -lallreduce
BOOST_LIBS = -lboost_system -lboost_program_options
CPPREST_LIBS = -lcpprest -lssl -lcrypto -pthread -ldl 
ALL_LIBS = $(VW_LIB) $(BOOST_LIBS) $(CPPREST_LIBS) $(LIBS)

FLAGS += -fPIC

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = $(shell find $(CURDIR)/../rlclientlib . -name '*.cc' ! -name '*_wrap.cc')
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(shell find $(CURDIR)/../rlclientlib $(CURDIR)/../include . -name '*.h')

INTERFACES = $(wildcard *.i)
WRAPPED_INTERFACES = $(patsubst %.i, %_wrap.cc, $(INTERFACES))
SOURCES += $(WRAPPED_INTERFACES)

%_wrap.cc: %.i
	swig -c++ -python -o $@ $<

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(FLAGS) -shared -o $@ $(OBJECTS) $(LIBDIR) $(ALL_LIBS)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f ../rlclientlib/*.o
	-rm -f *_wrap.cc
	-rm -f *.py
	-rm -f *.pyc