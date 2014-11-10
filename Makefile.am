SUBDIRS = vowpalwabbit cluster library

nobase_include_HEADERS = vowpalwabbit/allreduce.h \
	vowpalwabbit/comp_io.h \
	vowpalwabbit/config.h \
	vowpalwabbit/example.h \
	vowpalwabbit/global_data.h \
	vowpalwabbit/hash.h \
	vowpalwabbit/io_buf.h \
	vowpalwabbit/learner.h \
	vowpalwabbit/loss_functions.h \
	vowpalwabbit/parse_primitives.h \
	vowpalwabbit/parser.h \
	vowpalwabbit/simple_label.h \
	vowpalwabbit/v_array.h \
	vowpalwabbit/vw.h \
	vowpalwabbit/vwdll.h

noinst_HEADERS = vowpalwabbit/accumulate.h \
	vowpalwabbit/autolink.h \
	vowpalwabbit/bfgs.h \
	vowpalwabbit/binary.h \
	vowpalwabbit/bs.h \
	vowpalwabbit/cache.h \
	vowpalwabbit/cb.h \
	vowpalwabbit/cb_algs.h \
	vowpalwabbit/cbify.h \
	vowpalwabbit/comp_io.h \
	vowpalwabbit/constant.h \
	vowpalwabbit/cost_sensitive.h \
	vowpalwabbit/csoaa.h \
	vowpalwabbit/ect.h \
	vowpalwabbit/gd.h \
	vowpalwabbit/gd_mf.h \
	vowpalwabbit/kernel_svm.h \
	vowpalwabbit/lda_core.h \
	vowpalwabbit/log_multi.h \
	vowpalwabbit/lrq.h \
	vowpalwabbit/memory.h \
	vowpalwabbit/mf.h \
	vowpalwabbit/multiclass.h \
	vowpalwabbit/network.h \
	vowpalwabbit/nn.h \
	vowpalwabbit/noop.h \
	vowpalwabbit/oaa.h \
	vowpalwabbit/parse_args.h \
	vowpalwabbit/parse_example.h \
	vowpalwabbit/parse_regressor.h \
	vowpalwabbit/print.h \
	vowpalwabbit/rand48.h \
	vowpalwabbit/reductions.h \
	vowpalwabbit/scorer.h \
	vowpalwabbit/search.h \
	vowpalwabbit/search_sequencetask.h \
	vowpalwabbit/search_entityrelationtask.h \
	vowpalwabbit/search_hooktask.h \
	vowpalwabbit/search_multiclasstask.h \
	vowpalwabbit/search_dep_parser.h \
	vowpalwabbit/sender.h \
	vowpalwabbit/stagewise_poly.h \
	vowpalwabbit/topk.h \
	vowpalwabbit/unique_sort.h \
	vowpalwabbit/v_hashmap.h

ACLOCAL_AMFLAGS = -I acinclude.d

test: all
	cd test && ./RunTests -f -E 0.001 ../vowpalwabbit/vw ../vowpalwabbit/vw
