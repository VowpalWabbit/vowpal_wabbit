# Underscore is required to be consistent with Swig's name generation
TARGET = _rl_client.so

PIP_FOLDER = ./rl_client/

# Python 3.6 is specifically provided, tricky if we want to link against different versions.
INCLUDE = -I ../../include -I ./ $(shell python3.6-config --includes)
RLCLIENT_LIB = -L ../../rlclientlib -lrlclient
VW_LIB = -L ../../../vowpalwabbit -lvw -lallreduce
BOOST_LIBS = -lboost_system
CPPREST_LIBS = -lcpprest -lssl -lcrypto -pthread -ldl
ALL_LIBS = $(RLCLIENT_LIB) $(VW_LIB) $(BOOST_LIBS) $(CPPREST_LIBS) $(LIBS)

.PHONY: default all clean

default: pip
all: default

things: all

MODULE_NAME = rl_client
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

pip: $(MAIN_SWIG_WRAPPED_INTERFACE)
	cp $(MODULE_NAME).py $(PIP_FOLDER) & python3.6 setup.py bdist_wheel

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS) $(WRAPPED_INTERFACES)

$(TARGET): $(OBJECTS)
	$(CXX) $(FLAGS) -shared -o $@ $(OBJECTS) $(LIBDIR) $(ALL_LIBS)

test: $(TARGET)
	python3.6 -m unittest -v test

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f $(MODULE_NAME)_wrap.cc
	-rm -f $(MODULE_NAME)_wrap.h
	-rm -f $(MODULE_NAME).py
	-rm -f $(MODULE_NAME).pyc
	-rm -f $(PIP_FOLDER)/$(MODULE_NAME).py
	-python3.6 setup.py clean --all
	-rm -rf *.egg-info
	-rm -rf dist
	-rm -rf __pycache__
