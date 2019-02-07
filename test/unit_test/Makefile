TARGET = vw-unit-test.out

VW_LIB = -L../../vowpalwabbit -lvw -lallreduce
BOOST_LIBS = -lboost_unit_test_framework -lboost_system
ALL_LIBS = $(VW_LIB) $(BOOST_LIBS) $(LIBS)

INCLUDE = -I ../../vowpalwabbit -I ../../explore

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = $(wildcard *.cc)
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(info $$SOURCES is [${SOURCES}])
$(info $$OBJECTS is [${OBJECTS}])

$(TARGET): $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) $(LIBDIR) $(ALL_LIBS) -Wall -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
