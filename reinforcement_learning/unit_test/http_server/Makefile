TARGET = http_server
FLAGS = -std=c++11 $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE) -fPIC #-DVW_LDA_NO_SSE
LIBS = -lm -pthread -lboost_system -lcrypto -lssl -lcpprest
INCLUDE = -I ../include

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
