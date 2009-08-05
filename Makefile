COMPILER = g++
LIBS = -l boost_program_options -l pthread
BOOST_INCLUDE = /usr/local/boost/include/boost-1_34_1
BOOST_LIBRARY = /usr/local/boost/lib

ARCH = -march=nocona

# for normal fast execution.
FLAGS = -Wall $(ARCH) -ffast-math -fno-strict-aliasing -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -O3

# for parallelization
#FLAGS = -Wall $(ARCH) -ffast-math -Wno-strict-aliasing -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -O3 -fopenmp

# for profiling
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -pg -g

# for valgrind
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -g -O0

BINARIES = vw offset_tree

all:	$(BINARIES)

vw.o:	 parse_example.h  parse_regressor.h  parse_args.h  source.h

offset_tree.o:	parse_example.h parse_regressor.h parse_args.h source.h

parse_args.o:	 parse_regressor.h  parse_example.h  io.h  gd.h

parse_example.o:  io.h  parse_example.cc  source.h

cache.o:	 parse_example.h

sparse_dense.o:	 parse_example.h

gd.o:	 parse_example.h

%.o:	 %.cc  %.h
	$(COMPILER) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(COMPILER) $(FLAGS) -c $< -o $@

vw: hash.o  io.o  parse_regressor.o  parse_primitives.o  cache.o  parse_example.o  sparse_dense.o  parse_args.o  gd.o  source.o  vw.o loss_functions.o  main.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)

offset_tree: 	hash.o io.o parse_regressor.o parse_primitives.o cache.o sparse_dense.o parse_example.o parse_args.o gd.o source.o offset_tree.o loss_functions.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)


test: vw vw-train vw-test
	@echo $(UNAME)

vw-train: vw
	@echo "TEST: vw training ..."
	@cat test/train.dat | ./vw -b 24 -l 20 --initial_t 128000 \
	--power_t 1 -f test/t_r_temp

vw-test: vw-train
	@echo
	@echo "TEST: vw test only ..."
	@cat test/test.dat | ./vw -t -i test/t_r_temp -p test/t_p_out

install: vw
	cp $(BINARIES) ~/bin

clean:
	rm -f  *.o $(BINARIES) *~
