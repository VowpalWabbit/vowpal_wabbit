all:	spanning_tree

%.o:	 %.cc  %.h
	$(COMPILER) $(FLAGS) -c $< -o $@

%.o:	 %.cc
	$(COMPILER) $(FLAGS) -c $< -o $@

spanning_tree: spanning_tree.o
	$(COMPILER) $(FLAGS) -o $@ $+ 

install: spanning_tree
	cp spanning_tree /usr/local/bin

clean:
	rm -f  *.o $(BINARIES) *~ $(MANPAGES)
