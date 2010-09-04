COMPILER = g++
LIBS = -l boost_program_options -l pthread -l z
#LIBS = -l boost_program_options-gcc34 -l pthread -l z
BOOST_INCLUDE = /usr/local/boost/include/boost-1_34_1
BOOST_LIBRARY = /usr/local/boost/lib

ARCH = -march=nocona

# for normal fast execution.
#FLAGS = -Wall $(ARCH) -ffast-math -fno-strict-aliasing -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -O3

# for parallelization
#FLAGS = -Wall $(ARCH) -ffast-math -Wno-strict-aliasing -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -O3 -fopenmp

# for profiling
#FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -pg -g

# for valgrind
FLAGS = -Wall $(ARCH) -ffast-math -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) -g -O0

BINARIES = vw
MANPAGES = vw.1

all:	$(BINARIES) $(MANPAGES)
#all:	$(BINARIES)

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

vw: hash.o  global_data.o delay_ring.o message_relay.o io.o parse_regressor.o  parse_primitives.o unique_sort.o cache.o simple_label.o parse_example.o multisource.o sparse_dense.o  network.o parse_args.o  gd.o noop.o parser.o vw.o loss_functions.o sender.o main.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)

offset_tree: 	hash.o io.o parse_regressor.o parse_primitives.o cache.o sparse_dense.o parse_example.o parse_args.o gd.o parser.o offset_tree.o loss_functions.o
	$(COMPILER) $(FLAGS) -L$(BOOST_LIBRARY) -o $@ $+ $(LIBS)


test: vw vw-train vw-test
	@echo $(UNAME)

vw-train: vw
	@echo "TEST: vw training ..."
	@rm -f test/train.dat.cache
	@./vw -b 17 -l 20 --initial_t 128000 \
	--power_t 1 -f test/t_r_temp -c --passes 2 -d test/train.dat --compressed --ngram 3 --skips 1

vw-test: vw-train
	@echo
	@echo "TEST: vw test only ..."
	@cat test/test.dat | ./vw -t -i test/t_r_temp -p test/t_p_out

install: vw
	cp $(BINARIES) ~/bin

clean:
	rm -f  *.o $(BINARIES) *~ $(MANPAGES)
