TARGET = librlclient.a
ARFLAGS = rcs

INCLUDE = -I ../include -I ../../explore -I ../explore -I ../../rapidjson/include -I ./

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

generated/OutcomeEvent_generated.h: ./schema/OutcomeEvent.fbs
	flatc -o ./generated/ --cpp ./schema/OutcomeEvent.fbs

generated/RankingEvent_generated.h: ./schema/RankingEvent.fbs
	flatc -o ./generated/ --cpp ./schema/RankingEvent.fbs
	
SOURCES = $(filter-out main.cc, $(wildcard *.cc)) $(wildcard */*.cc)
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS) generated/RankingEvent_generated.h generated/OutcomeEvent_generated.h
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	ar $(ARFLAGS) $@ $(OBJECTS)

clean:
	-rm -f *.o
	-rm -f */*.o
	-rm -f $(TARGET)
	-rm -f ./generated/*

