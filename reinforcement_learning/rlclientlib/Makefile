TARGET = librlclient.a
ARFLAGS = rcs 

INCLUDE = -I ../include -I ../../explore -I ../explore -I ../../rapidjson/include -I ./

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = $(filter-out main.cc, $(wildcard *.cc)) $(wildcard */*.cc)
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	ar $(ARFLAGS) $@ $(OBJECTS) 

clean:
	-rm -f *.o
	-rm -f $(TARGET)