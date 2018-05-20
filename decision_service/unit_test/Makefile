TARGET = rlclient-test.out
LIBS = -lm -pthread -lboost_system -lcrypto -lssl -lcpprest -lrlclient -lboost_unit_test_framework
LIBDIR = -L ../rlclientlib
INCLUDE = -I ../include -I ../rlclientlib

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = $(filter-out ds_model_mgmt_test.cc, $(wildcard *.cc)) http_server/http_server.cc #$(wildcard */*.cc)
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBDIR) $(LIBS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)