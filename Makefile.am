SUBDIRS = src cluster

ACLOCAL_AMFLAGS = -I acinclude.d

test: all
	cd test && ./RunTests -f -E 0.001 ../src/vw ../src/vw