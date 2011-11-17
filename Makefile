COMPILER = g++
LIBS = -l boost_program_options -l pthread -l z

ARCH = $(shell test `g++ -v 2>&1 | tail -1 | cut -d ' ' -f 3 | cut -d '.' -f 1,2` \< 4.3 && echo -march=nocona || echo -march=native)

#LIBS = -l boost_program_options-gcc34 -l pthread -l z
BOOST_INCLUDE = /usr/include
#BOOST_INCLUDE = /usr/local/include
BOOST_LIBRARY = /usr/local/lib

OPTIM_FLAGS = -O3 -fomit-frame-pointer -ffast-math -fno-strict-aliasing
WARN_FLAGS = -Wall -pedantic #-Werror 

# for normal fast execution.
FLAGS = $(ARCH) $(WARN_FLAGS) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) #-DVW_LDA_NO_SSE

# for parallelization
#FLAGS = -Wall $(ARCH) -ffast-math -Wno-strict-aliasing -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -O3 -fopenmp

# for profiling
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -pg -g

# for valgrind
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -g -O0

BINARIES = vw active_interactor
MANPAGES = vw.1

#all:	$(BINARIES) $(MANPAGES)
all:	depend $(BINARIES)

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

depend:	*.cc
	gcc -MM *.cc > depend

-include depend

%.o:	 %.cc  %.h
	$(COMPILER) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(COMPILER) $(FLAGS) -c $< -o $@

export

spanning_tree: 
	cd cluster; $(MAKE); cd ..

vw: hash.o  global_data.o delay_ring.o io.o parse_regressor.o  parse_primitives.o unique_sort.o cache.o simple_label.o parse_example.o sparse_dense.o  network.o parse_args.o allreduce.o accumulate.o gd.o lda_core.o gd_mf.o bfgs.o noop.o parser.o vw.o loss_functions.o sender.o main.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)

active_interactor:	active_interactor.cc
	$(COMPILER) $(FLAGS) -o $@ $+

offset_tree: 	hash.o io.o parse_regressor.o parse_primitives.o cache.o sparse_dense.o parse_example.o parse_args.o gd.o parser.o offset_tree.o loss_functions.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)

.FORCE:

test: .FORCE
	@echo "vw running test-suite..."
	@(cd test && ./RunTests -f -E 0.001 ../vw ../vw)

install: $(BINARIES)
	cp $(BINARIES) /usr/local/bin; cd cluster; $(MAKE) install

clean:
	rm -f  *.o $(BINARIES) *~ $(MANPAGES); cd cluster; $(MAKE) clean; cd ..
