SUBDIRS = vowpalwabbit cluster library

nobase_include_HEADERS = vowpalwabbit/vw.h vowpalwabbit/allreduce.h

noinst_HEADERS = vowpalwabbit/accumulate.h vowpalwabbit/autolink.h	\
	vowpalwabbit/beam.h vowpalwabbit/bfgs.h vowpalwabbit/binary.h	\
	vowpalwabbit/bs.h vowpalwabbit/cache.h vowpalwabbit/cb.h	\
	vowpalwabbit/comp_io.h vowpalwabbit/constant.h			\
	vowpalwabbit/csoaa.h vowpalwabbit/ect.h vowpalwabbit/example.h	\
	vowpalwabbit/gd.h vowpalwabbit/mf.h				\
	vowpalwabbit/global_data.h vowpalwabbit/hash.h			\
	vowpalwabbit/io_buf.h vowpalwabbit/lda_core.h			\
	vowpalwabbit/learner.h vowpalwabbit/loss_functions.h		\
	vowpalwabbit/network.h vowpalwabbit/nn.h vowpalwabbit/noop.h	\
	vowpalwabbit/oaa.h vowpalwabbit/parse_args.h			\
	vowpalwabbit/parse_example.h vowpalwabbit/parse_primitives.h	\
	vowpalwabbit/parse_regressor.h vowpalwabbit/parser.h		\
	vowpalwabbit/rand48.h vowpalwabbit/searn.h			\
	vowpalwabbit/searn_sequencetask.h vowpalwabbit/sender.h		\
	vowpalwabbit/simple_label.h vowpalwabbit/sparse_dense.h		\
	vowpalwabbit/topk.h vowpalwabbit/unique_sort.h			\
	vowpalwabbit/v_array.h vowpalwabbit/v_hashmap.h			\
	vowpalwabbit/wap.h

ACLOCAL_AMFLAGS = -I acinclude.d

test: all
	cd test && ./RunTests -f -E 0.001 ../vowpalwabbit/vw ../vowpalwabbit/vw
