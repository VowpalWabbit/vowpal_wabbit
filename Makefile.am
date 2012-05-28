SUBDIRS = vowpalwabbit cluster

nobase_include_HEADERS = vowpalwabbit/accumulate.h vowpalwabbit/comp_io.h vowpalwabbit/gd.h vowpalwabbit/io.h vowpalwabbit/noop.h vowpalwabbit/parse_primitives.h \
	vowpalwabbit/simple_label.h vowpalwabbit/vw.h vowpalwabbit/allreduce.h vowpalwabbit/config.h vowpalwabbit/gd_mf.h vowpalwabbit/lda_core.h vowpalwabbit/oaa.h \
	vowpalwabbit/parse_regressor.h vowpalwabbit/sparse_dense.h vowpalwabbit/bfgs.h vowpalwabbit/constant.h vowpalwabbit/global_data.h vowpalwabbit/loss_functions.h \
	vowpalwabbit/parse_args.h vowpalwabbit/parser.h vowpalwabbit/unique_sort.h vowpalwabbit/cache.h vowpalwabbit/example.h vowpalwabbit/hash.h vowpalwabbit/network.h \
	vowpalwabbit/parse_example.h vowpalwabbit/sender.h vowpalwabbit/v_array.h vowpalwabbit/wap.h vowpalwabbit/sequence.h vowpalwabbit/csoaa.h vowpalwabbit/ect.h

ACLOCAL_AMFLAGS = -I acinclude.d

test: all
	cd test && ./RunTests -f -E 0.001 ../vowpalwabbit/vw ../vowpalwabbit/vw