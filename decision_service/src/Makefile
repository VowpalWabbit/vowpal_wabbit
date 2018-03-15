TARGET = ds_driver
LIBS = -lm
CC = g++
CXXFLAGS = -g -Wall -std=c++11
LDLIBS += -pthread -lboost_system -lcrypto -lssl -lcpprest

JSON_INCLUDE = -I ../../rapidjson/include

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CC) $(CXXFLAGS) $(JSON_INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(JSON_INCLUDE) $(LDLIBS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
