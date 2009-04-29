ARCH = -msse2 -mfpmath=sse -march=pentium-m
FLAGS = -Wall $(ARCH) -O3 -ffast-math -D_FILE_OFFSET_BITS=64 -Wno-deprecated -I /usr/include/boost-1_34_1 

all:	vw install

vw.o:	vw.cc parse_example.h parse_regressor.h parse_args.h
	g++ $(FLAGS) -c $< -o $@

parse_args.o:	parse_regressor.h parse_example.h io.h

parse_example.o:	io.h stack.h parse_example.cc

io.o:	stack.h

%.o:	%.cc %.h
	g++ $(FLAGS) -c $< -o $@

vw: hash.o io.o parse_regressor.o parse_example.o parse_args.o vw.o 
	g++ $(FLAGS) -l boost_program_options -l pthread -o vw $+

install:
	cp vw ~/bin

clean:
	rm -f *.o vw *~
