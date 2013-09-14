SUBDIRS = vowpalwabbit cluster library

nobase_include_HEADERS = vowpalwabbit/vw.h vowpalwabbit/allreduce.h

noinst_HEADERS = vowpalwabbit/accumulate.h vowpalwabbit/oaa.h		\
       vowpalwabbit/autolink.h vowpalwabbit/binary.h			\
       vowpalwabbit/parse_args.h vowpalwabbit/bfgs.h			\
       vowpalwabbit/parse_example.h vowpalwabbit/cache.h		\
       vowpalwabbit/parse_primitives.h vowpalwabbit/comp_io.h		\
       vowpalwabbit/parse_regressor.h vowpalwabbit/constant.h		\
       vowpalwabbit/parser.h vowpalwabbit/csoaa.h vowpalwabbit/beam.h	\
       vowpalwabbit/searn.h vowpalwabbit/ect.h vowpalwabbit/cb.h	\
       vowpalwabbit/searn_sequencetask.h vowpalwabbit/example.h		\
       vowpalwabbit/sender.h vowpalwabbit/gd.h vowpalwabbit/gd_mf.h	\
       vowpalwabbit/simple_label.h vowpalwabbit/global_data.h		\
       vowpalwabbit/sparse_dense.h vowpalwabbit/hash.h			\
       vowpalwabbit/unique_sort.h vowpalwabbit/io_buf.h			\
       vowpalwabbit/v_array.h vowpalwabbit/lda_core.h			\
       vowpalwabbit/v_hashmap.h vowpalwabbit/loss_functions.h		\
       vowpalwabbit/network.h vowpalwabbit/wap.h vowpalwabbit/noop.h	\
       vowpalwabbit/nn.h vowpalwabbit/learner.h vowpalwabbit/bs.h

ACLOCAL_AMFLAGS = -I acinclude.d

test: all
	cd test && ./RunTests -f -E 0.001 ../vowpalwabbit/vw ../vowpalwabbit/vw
