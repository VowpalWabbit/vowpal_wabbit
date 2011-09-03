COMPILER = g++
LIBS = -l boost_program_options -l pthread -l z

#LIBS = -l boost_program_options-gcc34 -l pthread -l z
BOOST_INCLUDE = /usr/include
#BOOST_INCLUDE = /usr/local/include
BOOST_LIBRARY = /usr/local/lib

ARCH = -march=nocona 
OPTIM_FLAGS = -std=c++0x -O3 -fomit-frame-pointer -ffast-math -fno-strict-aliasing 
WARN_FLAGS = -Wall #-Werror 

# for normal fast execution.
FLAGS = $(ARCH) $(WARN_FLAGS) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) #-DVW_LDA_NO_SSE

# for parallelization
#FLAGS = -Wall $(ARCH) -ffast-math -Wno-strict-aliasing -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -O3 -fopenmp

# for profiling
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -pg -g

# for valgrind
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -g -O0

BINARIES = spanning_tree

all:	$(BINARIES)

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

%.o:	 %.cc  %.h
	$(COMPILER) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(COMPILER) $(FLAGS) -c $< -o $@

spanning_tree: spanning_tree.o
	$(COMPILER) $(FLAGS) -o $@ $+ 

.FORCE:

test: .FORCE
	@echo "vw running test-suite..."
	@(cd test && ./RunTests -f -E 0.001 ../vw ../lda)

install: allreduce_master
	cp $(BINARIES) /usr/local/bin

clean:
	rm -f  *.o $(BINARIES) *~ $(MANPAGES)
