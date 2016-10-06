CXX = $(shell which g++)
ifneq ($(CXX),)
  #$(warning Using clang: "$(CXX)")
  ARCH = -D__extern_always_inline=inline
else
  CXX = g++
#  $(warning Using g++)
ARCH = $(shell test `g++ -v 2>&1 | tail -1 | cut -d ' ' -f 3 | cut -d '.' -f 1,2` \< 4.3 && echo -march=nocona || echo -march=native)
endif

ifeq ($(CXX),)
  $(error No compiler found)
endif

FLAGS += -I ../vowpalwabbit 
STDLIBS = $(BOOST_LIBRARY) $(LIBS)

all:	spanning_tree

%.o:	 %.cc  %.h
	$(CXX) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(CXX) $(FLAGS) -c $< -o $@

spanning_tree: spanning_tree_main.o ../vowpalwabbit/spanning_tree.o ../vowpalwabbit/vw_exception.o
	$(CXX) $(FLAGS) -o $@ $+ $(STDLIBS) 

install: spanning_tree
	cp spanning_tree /usr/local/bin

clean:
	rm -f  *.o $(BINARIES) *~ $(MANPAGES)
