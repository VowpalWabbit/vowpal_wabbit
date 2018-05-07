TARGET = dsclientlib
LIBS = -lm
CXX = g++
CXXFLAGS = -g -Wall -std=c++11
LIBS += -pthread -lboost_system -lcrypto -lssl -lcpprest

INCLUDE = -I ../include

.PHONY: default all clean

default: $(TARGET)
all: default

SOURCES = $(wildcard *.cc) $(wildcard */*.cc)
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)