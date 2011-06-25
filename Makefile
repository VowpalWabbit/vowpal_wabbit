COMPILER = g++
LIBS = -l boost_program_options -l pthread -l z
#LIBS = -l boost_program_options-gcc34 -l pthread -l z
BOOST_INCLUDE = /usr/local/include
BOOST_LIBRARY = /usr/local/lib

ARCH = -march=nocona
OPTIM_FLAGS = -O3 -fomit-frame-pointer -ffast-math -fno-strict-aliasing
WARN_FLAGS = -Wall -Werror 

# for normal fast execution.
FLAGS = $(ARCH) $(WARN_FLAGS) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE)

# for parallelization
#FLAGS = -Wall $(ARCH) -ffast-math -Wno-strict-aliasing -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -O3 -fopenmp

# for profiling
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -pg -g

# for valgrind
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -g -O0

BINARIES = vw allreduce_master active_interactor lda
MANPAGES = vw.1

#all:	$(BINARIES) $(MANPAGES)
all:	$(BINARIES)

%.1:	%
	help2man --no-info --name="Vowpal Wabbit -- fast online learning tool" ./$< > $@

vw.o:	 parse_example.h  parse_regressor.h  parse_args.h  parser.h

offset_tree.o:	parse_example.h parse_regressor.h parse_args.h parser.h

parse_args.o:	 parse_regressor.h  parse_example.h  io.h  comp_io.h gd.h

parse_example.o:  io.h  comp_io.h  parse_example.cc  parser.h

sender.o: parse_example.h

cache.o:	 parser.h

sparse_dense.o:	 parse_example.h

gd.o:	 parse_example.h

%.o:	 %.cc  %.h
	$(COMPILER) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(COMPILER) $(FLAGS) -c $< -o $@

allreduce_master: allreduce_master.o
	$(COMPILER) $(FLAGS) -o $@ $+ 

vw: hash.o  global_data.o delay_ring.o message_relay.o io.o parse_regressor.o  parse_primitives.o unique_sort.o cache.o simple_label.o parse_example.o multisource.o sparse_dense.o  network.o parse_args.o gd.o allreduce.o cg.o noop.o parser.o vw.o loss_functions.o sender.o main.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)

lda: hash.o  global_data.o delay_ring.o message_relay.o io.o parse_regressor.o  parse_primitives.o unique_sort.o cache.o simple_label.o parse_example.o multisource.o sparse_dense.o  network.o parse_args.o gd.o lda_core.o noop.o parser.o loss_functions.o sender.o lda.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)

active_interactor:	active_interactor.cc
	$(COMPILER) $(FLAGS) -o $@ $+

offset_tree: 	hash.o io.o parse_regressor.o parse_primitives.o cache.o sparse_dense.o parse_example.o parse_args.o gd.o parser.o offset_tree.o loss_functions.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)

.FORCE:

test: .FORCE
	@echo "vw running test-suite..."
	@(cd test && ./RunTests ../vw ../lda)

install: vw
	cp $(BINARIES) /usr/local/bin

clean:
	rm -f  *.o $(BINARIES) *~ $(MANPAGES)
