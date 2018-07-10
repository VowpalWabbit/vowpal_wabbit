
TARGET = libvwslim.a
ARFLAGS = rcs 

INCLUDE = -I ../vowpalwabbit -I ../explore -I ./

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = example_predict_builder.cc model_parser.cc opts.cc vw_slim_predict.cc ../vowpalwabbit/vw_exception.cc
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
