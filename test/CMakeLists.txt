if(BUILD_TESTING)
    set(test_dirs train-sets test-sets pred-sets model-sets)
    foreach(test_dir IN LISTS test_dirs)
        if(EXISTS    ${CMAKE_CURRENT_BINARY_DIR}/${test_dir})
            continue()
        endif()

        file(COPY ${test_dir} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    endforeach()

    set(VW $<TARGET_FILE:vowpal_wabbit>)

    add_test(NAME vw_1
        COMMAND ${VW} -k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat
        -f models/0001_1.model -c --passes 8 --invariant
        --ngram 3 --skips 1 --holdout_off
            train-sets/ref/0001.stderr

    # Test 2: checking predictions as well
    )

    add_test(NAME vw_2
        COMMAND ${VW} -k -t -d train-sets/0001.dat -i models/0001_1.model -p 0001.predict --invariant
        test-sets/ref/0001.stderr
        pred-sets/ref/0001.predict

    # Test 3: without -d, training only
    )

    add_test(NAME vw_3
        COMMAND ${VW} -k -d train-sets/0002.dat -f models/0002.model --invariant
        train-sets/ref/0002.stderr

    # Test 4: same, with -d
    )

    add_test(NAME vw_4
        COMMAND ${VW} -k -d train-sets/0002.dat -f models/0002.model --invariant
        train-sets/ref/0002.stdout
        train-sets/ref/0002.stderr

    # Test 5: add -q .., adaptive, and more (same input, different outputs)
    )

    add_test(NAME vw_5
        COMMAND ${VW} -k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model -d train-sets/0002.dat
        train-sets/ref/0002a.stderr

    # Test 6: run predictions on Test 4 model
    # Pretending the labels aren't there
    )

    add_test(NAME vw_6
        COMMAND ${VW} -k -t -i models/0002.model -d train-sets/0002.dat -p 0002b.predict
        test-sets/ref/0002b.stderr
        pred-sets/ref/0002b.predict

    # Test 7: using normalized adaptive updates and a low --power_t
    )

    add_test(NAME vw_7
        COMMAND ${VW} -k --power_t 0.45 -f models/0002c.model -d train-sets/0002.dat
        train-sets/ref/0002c.stderr

    # Test 8: predicts on test 7 model
    )

    add_test(NAME vw_8
        COMMAND ${VW} -k -t -i models/0002c.model -d train-sets/0002.dat -p 0002c.predict
        test-sets/ref/0002c.stderr
        pred-sets/ref/0002c.predict

    # Test 9: label-dependent features with csoaa_ldf
    )

    add_test(NAME vw_9
        COMMAND ${VW} -k -c -d train-sets/cs_test.ldf -p cs_test.ldf.csoaa.predict --passes 10 --invariant --csoaa_ldf multiline --holdout_off --noconstant
        train-sets/ref/cs_test.ldf.csoaa.stderr
        train-sets/ref/cs_test.ldf.csoaa.predict

    # Test 10: label-dependent features with wap_ldf
    )

    add_test(NAME vw_10
        COMMAND ${VW} -k -c -d train-sets/cs_test.ldf -p cs_test.ldf.wap.predict --passes 10 --invariant --wap_ldf multiline --holdout_off --noconstant
        train-sets/ref/cs_test.ldf.wap.stderr
        train-sets/ref/cs_test.ldf.wap.predict

    # Test 11: one-against-all
    )

    add_test(NAME vw_11
        COMMAND ${VW} -k --oaa 10 -c --passes 10 -d train-sets/multiclass --holdout_off
        train-sets/ref/oaa.stderr

    # Test 12: Error Correcting Tournament
    )

    add_test(NAME vw_12
        COMMAND ${VW} -k --ect 10 --error 3 -c --passes 10 --invariant -d train-sets/multiclass --holdout_off
        train-sets/ref/multiclass.stderr

    # Test 13: Run search (dagger) on wsj_small for 6 passes extra features
    )

    add_test(NAME vw_13
        COMMAND ${VW} -k -c -d train-sets/wsj_small.dat.gz --passes 6 
        --search_task sequence --search 45 --search_alpha 1e-6 
        --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1 
        --holdout_off
            train-sets/ref/search_wsj.stderr

    # Test 14: Run search (searn) on wsj_small for 6 passes extra features
    )

    add_test(NAME vw_14
        COMMAND ${VW} -k -c -d train-sets/wsj_small.dat.gz --passes 6 
        --search_task sequence --search 45 --search_alpha 1e-6 
        --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1 
        --holdout_off --search_passes_per_policy 3 --search_interpolation policy
            train-sets/ref/search_wsj2.dat.stdout
            train-sets/ref/search_wsj2.dat.stderr

    # Test 15: LBFGS on zero derivative input
    )

    add_test(NAME vw_15
        COMMAND ${VW} -k -c -d train-sets/zero.dat --loss_function=squared -b 20 --bfgs --mem 7 --passes 5 --l2 1.0 --holdout_off
        train-sets/ref/zero.stdout
        train-sets/ref/zero.stderr

    # Test 16: LBFGS early termination
    )

    add_test(NAME vw_16
        COMMAND ${VW} -k -c -d train-sets/rcv1_small.dat --loss_function=logistic --bfgs --mem 7 --passes 20 --termination 0.001 --l2 1.0 --holdout_off
        train-sets/ref/rcv1_small.stdout
        train-sets/ref/rcv1_small.stderr

    # Test 17: Run LDA with 100 topics on 1000 Wikipedia articles
    )

    add_test(NAME vw_17
        COMMAND ${VW} -k --lda 100 --lda_alpha 0.01 --lda_rho 0.01 --lda_D 1000 -l 1 -b 13 --minibatch 128 -d train-sets/wiki256.dat
        train-sets/ref/wiki1K.stderr

    # Test 18: Run search on seq_small for 12 passes, 4 passes per policy
    )

    add_test(NAME vw_18
        COMMAND ${VW} -k -c -d train-sets/seq_small --passes 12 --invariant --search 4 --search_task sequence --holdout_off
        train-sets/ref/search_small.stderr

    # Test 19: neural network 3-parity with 2 hidden units
    )

    add_test(NAME vw_19
        COMMAND ${VW} -k -c -d train-sets/3parity --hash all --passes 3000 -b 16 --nn 2 -l 10 --invariant -f models/0021.model --random_seed 19 --quiet --holdout_off
        train-sets/ref/3parity.stderr

    # Test 20: neural network 3-parity with 2 hidden units (predict)
    )

    add_test(NAME vw_20
        COMMAND ${VW} -d train-sets/3parity -t -i models/0021.model -p 0022.predict
        pred-sets/ref/0022.stderr
        pred-sets/ref/0022.predict

    # Test 21: cubic features -- on a parity test case
    )

    add_test(NAME vw_21
        COMMAND ${VW} -k -c -f models/xxor.model -d train-sets/xxor.dat --cubic abc --passes 100 --holdout_off --progress 1.33333
        train-sets/ref/xxor.stderr

    # Test 22: matrix factorization -- training
    )

    add_test(NAME vw_22
        COMMAND ${VW} -k -d train-sets/ml100k_small_train -b 16 -q ui --rank 10 
        --l2 2e-6 --learning_rate 0.05 --passes 2 
        --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg 
        -c --loss_function classic --holdout_off
            train-sets/ref/ml100k_small.stdout
            train-sets/ref/ml100k_small.stderr

    # Test 23: matrix factorization -- testing
    )

    add_test(NAME vw_23
        COMMAND ${VW} -i models/movielens.reg -t -d test-sets/ml100k_small_test
        test-sets/ref/ml100k_small.stdout
        test-sets/ref/ml100k_small.stderr

    # Test 24: active-learning -- training
    )

    add_test(NAME vw_24
        COMMAND ${VW} -k --active --simulation --mellowness 0.000001 -d train-sets/rcv1_small.dat -l 10 --initial_t 10 --random_seed 3
        train-sets/ref/active-simulation.t24.stderr

    # Test 25: bagging -- training regressor
    )

    add_test(NAME vw_25
        COMMAND ${VW} -k -d train-sets/0002.dat -f models/bs.reg.model --bootstrap 4 -p bs.reg.predict
        train-sets/ref/bs.reg.stderr
        train-sets/ref/bs.reg.predict

    # Test 26: bagging -- predicting with bagged regressor
    )

    add_test(NAME vw_26
        COMMAND ${VW} -d train-sets/0002.dat -i models/bs.reg.model -p bs.prreg.predict -t
        train-sets/ref/bs.prreg.stderr
        train-sets/ref/bs.prreg.predict

    # Test 27: bagging -- binary classifiers
    )

    add_test(NAME vw_27
        COMMAND ${VW} -d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict
        train-sets/ref/bs.vote.stderr
        train-sets/ref/bs.vote.predict

    # Test 28: bagging -- predict with bagged classifier
    )

    add_test(NAME vw_28
        COMMAND ${VW} -d train-sets/0001.dat -i models/bs.vote.model -p bs.prvote.predict -t
        train-sets/ref/bs.prvote.stderr
        train-sets/ref/bs.prvote.predict

    # Test 29: affix features
    )

    add_test(NAME vw_29
        COMMAND ${VW} -d train-sets/affix_test.dat -k -c --passes 10 --holdout_off --affix -2
        train-sets/ref/affix_test.stderr

    # Test 30: train --l1 regularized model
    )

    add_test(NAME vw_30
        COMMAND ${VW} -d train-sets/0001.dat -f models/mask.model --invert_hash mask.predict --l1 0.01
        train-sets/ref/mask.stderr

    # Test 31: train model using --feature_mask
    )

    add_test(NAME vw_31
        COMMAND ${VW} -d train-sets/0001.dat --invert_hash remask.predict --feature_mask models/mask.model -f models/remask.model
        train-sets/ref/remask.stderr

    # Test 32: train model using --feature_mask and --initial_regressor
    )

    add_test(NAME vw_32
        COMMAND ${VW} -d train-sets/0001.dat --feature_mask models/mask.model -i models/remask.model
        train-sets/ref/remask.final.stderr

    # Test 33: train model for topk recommender
    )

    add_test(NAME vw_33
        COMMAND ${VW} -d train-sets/topk.vw -f topk.model -q MF --passes 100 --cache_file topk-train.cache -k --holdout_off
        train-sets/ref/topk-train.stderr

    # Test 34: train model for topk recommender
    )

    add_test(NAME vw_34
        COMMAND ${VW} -P 1 -d train-sets/topk.vw -i topk.model --top 2 -p topk-rec.predict
        train-sets/ref/topk-rec.stderr
        train-sets/ref/topk-rec.predict

    # Test 35: non-centered data-set where constant >> 0
    #   To test the new --constant option without which performance is very weak
    )

    add_test(NAME vw_35
        COMMAND ${VW} -k --passes 100 -c --holdout_off --constant 1000 -d train-sets/big-constant.dat
        train-sets/ref/big-constant.stderr

    # Test 36: new option: --progress w/ integer arg
    )

    add_test(NAME vw_36
        COMMAND ${VW} -k -d train-sets/0001.dat --progress 10
        train-sets/ref/progress-10.stderr

    # Test 37: new-option: --progress w/ floating-point arg
    #           + alternate short form (-P)
    )

    add_test(NAME vw_37
        COMMAND ${VW} -k -d train-sets/0001.dat -P 0.5
        train-sets/ref/progress-0.5.stderr

    # Test 38: --nn without --quiet to avoid nn regressions
    #   (Needs to be a simple test, not one sensitive to symmetry breaking)
    )

    add_test(NAME vw_38
        COMMAND ${VW} -k -d train-sets/0001.dat --nn 1
        train-sets/ref/nn-1-noquiet.stderr

    # Test 39: cb with dr
    )

    add_test(NAME vw_39
        COMMAND ${VW} -d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dr --ngram 2 --skips 4 -b 24 -l 0.25
        train-sets/ref/rcv1_raw_cb_dr.stderr

    # Test 40: cb with ips
    )

    add_test(NAME vw_40
        COMMAND ${VW} -d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type ips --ngram 2 --skips 4 -b 24 -l 0.125
        train-sets/ref/rcv1_raw_cb_ips.stderr

    # Test 41: cb with dm
    )

    add_test(NAME vw_41
        COMMAND ${VW} -d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dm --ngram 2 --skips 4 -b 24 -l 0.125
        train-sets/ref/rcv1_raw_cb_dm.stderr

    # Test 42: --lda --passes 2 hang regression
    )

    add_test(NAME vw_42
        COMMAND ${VW} -k -d train-sets/lda-2pass-hang.dat --lda 10 -c --passes 2 --holdout_off
        train-sets/ref/lda-2pass-hang.stderr

    # Test 43: search sequence labeling, non-ldf train
    )

    add_test(NAME vw_43
        COMMAND ${VW} -k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequence --search 5 --holdout_off -f models/sequence_data.model
        train-sets/ref/sequence_data.nonldf.train.stderr

    # Test 44: search sequence labeling, non-ldf test
    )

    add_test(NAME vw_44
        COMMAND ${VW} -d train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.test.predict
        train-sets/ref/sequence_data.nonldf.test.stderr
        train-sets/ref/sequence_data.nonldf.test.predict

    # Test 45: make sure that history works
    )

    add_test(NAME vw_45
        COMMAND ${VW} -k -c -d train-sets/seq_small2 --passes 4 --search 4 --search_task sequence --holdout_off
        train-sets/ref/search_small2.stderr

    # Test 46: search sequence labeling, ldf train
    )

    add_test(NAME vw_46
        COMMAND ${VW} -k -c -d train-sets/sequence_data --passes 20 
        --search_rollout ref --search_alpha 1e-8 
        --search_task sequence_demoldf --csoaa_ldf m --search 5 
        --holdout_off -f models/sequence_data.ldf.model --noconstant
            train-sets/ref/sequence_data.ldf.train.stderr

    # Test 47: search sequence labeling, ldf test
    )

    add_test(NAME vw_47
        COMMAND ${VW} -d train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.test.predict --noconstant
        train-sets/ref/sequence_data.ldf.test.stderr
        train-sets/ref/sequence_data.ldf.test.predict

    # Test 48: search sequence SPAN labeling BIO, non-ldf train, no rollouts
    )

    add_test(NAME vw_48
        COMMAND ${VW} -k -c -d train-sets/sequencespan_data --passes 20 --invariant 
        --search_rollout none --search_task sequencespan --search 7 
        --holdout_off -f models/sequencespan_data.model
            train-sets/ref/sequencespan_data.nonldf.train.stderr

    # Test 49: search sequence SPAN labeling BIO, non-ldf test
    )

    add_test(NAME vw_49
        COMMAND ${VW} -d train-sets/sequencespan_data -t -i models/sequencespan_data.model -p sequencespan_data.nonldf.test.predict
        train-sets/ref/sequencespan_data.nonldf.test.stderr
        train-sets/ref/sequencespan_data.nonldf.test.predict

    # Test 50: search sequence SPAN labeling BILOU, non-ldf train
    )

    add_test(NAME vw_50
        COMMAND ${VW} -k -c -d train-sets/sequencespan_data --passes 20 --invariant 
        --search_rollout ref --search_alpha 1e-8 --search_task sequencespan 
        --search_span_bilou --search 7 --holdout_off 
        -f models/sequencespan_data.model
            train-sets/ref/sequencespan_data.nonldf-bilou.train.stderr

    # Test 51: search sequence SPAN labeling BILOU, non-ldf test
    )

    add_test(NAME vw_51
        COMMAND ${VW} -d train-sets/sequencespan_data -t --search_span_bilou -i models/sequencespan_data.model -p sequencespan_data.nonldf-bilou.test.predict
        train-sets/ref/sequencespan_data.nonldf-bilou.test.stderr
        train-sets/ref/sequencespan_data.nonldf-bilou.test.predict

    # Test 52: silly test for "argmax" task
    )

    add_test(NAME vw_52
        COMMAND ${VW} -d train-sets/argmax_data -k -c --passes 20 --search_rollout ref --search_alpha 1e-8 --search_task argmax --search 2 --holdout_off
        train-sets/ref/argmax_data.stderr

    # Test 53: (holdout-broken regression)
    # ensure we have no holdout loss of '0 h'
    )

    add_test(NAME vw_53
        COMMAND ${VW} -k -c --passes 2 -d train-sets/0001.dat
        train-sets/ref/holdout-loss-not-zero.stderr

    # Test 54: stagewise poly with exponent 0.25
    ####in the following stage_poly tests, there are minute differences in losses, which are not being fuzzy-diffed;
    ####thus the stderr is cleared (--quiet) and only comparing (fuzzy-diffed) predictions.
    )

    add_test(NAME vw_54
        COMMAND ${VW} --stage_poly --sched_exponent 0.25 --batch_sz 1000 --batch_sz_no_doubling -d train-sets/rcv1_small.dat -p stage_poly.s025.predict --quiet
        train-sets/ref/stage_poly.s025.stderr
        train-sets/ref/stage_poly.s025.predict

    # Test 55: stagewise poly with exponent 1.0
    )

    add_test(NAME vw_55
        COMMAND ${VW} --stage_poly --sched_exponent 1.0 --batch_sz 1000 --batch_sz_no_doubling -d train-sets/rcv1_small.dat --quiet
        train-sets/ref/stage_poly.s100.stderr

    # Test 56: stagewise poly with exponent 0.25 and doubling batches
    )

    add_test(NAME vw_56
        COMMAND ${VW} --stage_poly --sched_exponent 0.25 --batch_sz 1000 -d train-sets/rcv1_small.dat -p stage_poly.s025.doubling.predict --quiet
        train-sets/ref/stage_poly.s025.doubling.stderr
        train-sets/ref/stage_poly.s025.doubling.predict

    # Test 57: stagewise poly with exponent 1.0 and doubling batches
    )

    add_test(NAME vw_57
        COMMAND ${VW} --stage_poly --sched_exponent 1.0 --batch_sz 1000 -d train-sets/rcv1_small.dat -p stage_poly.s100.doubling.predict --quiet
        train-sets/ref/stage_poly.s100.doubling.stderr
        train-sets/ref/stage_poly.s100.doubling.predict

    # Test 58: library test, train the initial model
    )

    add_test(NAME vw_58
        COMMAND ${VW} -c -k -d train-sets/library_train -f models/library_train.w -q st --passes 100 --hash all --noconstant --csoaa_ldf m --holdout_off
        train-sets/ref/library_train.stdout
        train-sets/ref/library_train.stderr

    # Test 59: library test, run ezexample_predict
    ../library/ezexample_predict models/library_train.w
        train-sets/ref/ezexample_predict.stdout
        train-sets/ref/ezexample_predict.stderr

    # Test 60: empty test, bad builds (without make clean)
    # sometimes cause a SEGV even on empty input
    echo "" | )

    add_test(NAME vw_59
        COMMAND ${VW}
        train-sets/ref/empty-set.stderr

    # Test 61: daemon test
    ./daemon-test.sh
        test-sets/ref/vw-daemon.stdout

    # Test 62: SVM linear kernel
    )

    add_test(NAME vw_60
        COMMAND ${VW} --ksvm --l2 1 --reprocess 5 -b 18 -p ksvm_train.linear.predict -d train-sets/rcv1_smaller.dat
        train-sets/ref/ksvm_train.linear.stderr
        train-sets/ref/ksvm_train.linear.predict

    # Test 63: SVM polynomial kernel
    )

    add_test(NAME vw_61
        COMMAND ${VW} --ksvm --l2 1 --reprocess 5 -b 18 --kernel poly -p ksvm_train.poly.predict -d train-sets/rcv1_smaller.dat
        train-sets/ref/ksvm_train.poly.stderr
        train-sets/ref/ksvm_train.poly.predict

    # Test 64: SVM rbf kernel
    )

    add_test(NAME vw_62
        COMMAND ${VW} --ksvm --l2 1 --reprocess 5 -b 18 --kernel rbf -p ksvm_train.rbf.predict -d train-sets/rcv1_smaller.dat
        train-sets/ref/ksvm_train.rbf.stderr
        train-sets/ref/ksvm_train.rbf.predict

    # Test 65: Run search (dagger) on an entity-relation recognitions data set,
    # er_small, for 6 passes with constraints
    )

    add_test(NAME vw_63
        COMMAND ${VW} -k -c -d train-sets/er_small.vw --passes 6 --search_task entity_relation --search 10 --constraints --search_alpha 1e-8
        train-sets/ref/search_er.stderr

    # Test 66: Train a depenency parser with search (dagger)
    # on wsj_small.dparser.vw.gz for 6 passes
    )

    add_test(NAME vw_64
        COMMAND ${VW} -k -c -d train-sets/wsj_small.dparser.vw.gz --passes 6 --search_task dep_parser --search 12  --search_alpha 1e-4 --search_rollout oracle --holdout_off
        train-sets/ref/search_dep_parser.stderr

    # Test 67: classification with data from dictionaries
    # (eg embeddings or gazetteers) -- note that this is impossible without
    # dictionaries because --ignore w; also test to make sure gzipped dicts
    # work and dictionary redundancy checking works
    )

    add_test(NAME vw_65
        COMMAND ${VW} -k -c -d train-sets/dictionary_test.dat --binary --ignore w --holdout_off --passes 32 --dictionary w:dictionary_test.dict --dictionary w:dictionary_test.dict.gz --dictionary_path train-sets
        train-sets/ref/dictionary_test.stderr

    # Test 68: Search for multiclass classification
    )

    add_test(NAME vw_66
        COMMAND ${VW} -k -c -d train-sets/multiclass.sch --passes 20 --search_task multiclasstask --search 10 --search_alpha 1e-4 --holdout_off
        train-sets/ref/search_multiclass.stderr

    # Test 69: (see Test 43/Test 44): search sequence labeling, with selective branching
    )

    add_test(NAME vw_67
        COMMAND ${VW} -d train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10
        train-sets/ref/sequence_data.nonldf.beam.test.stderr
        train-sets/ref/sequence_data.nonldf.beam.test.predict

    # Test 70: (see Test 46/47) search sequence labeling, ldf test, with selective branching
    )

    add_test(NAME vw_68
        COMMAND ${VW} -d train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10 --noconstant
        train-sets/ref/sequence_data.ldf.beam.test.stderr
        train-sets/ref/sequence_data.ldf.beam.test.predict

    # Test 71: autolink
    )

    add_test(NAME vw_69
        COMMAND ${VW} -d train-sets/0002.dat --autolink 1 --examples 100 -p 0002.autolink.predict
        train-sets/ref/0002.autolink.stderr
        train-sets/ref/0002.autolink.predict

    # Test 72: train FTRL-Proximal
    )

    add_test(NAME vw_70
        COMMAND ${VW} -k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 1 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2
        train-sets/ref/0001_ftrl.stderr

    # Test 73: test FTRL-Proximal
    )

    add_test(NAME vw_71
        COMMAND ${VW} -k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl.predict
        test-sets/ref/0001_ftrl.stderr
        pred-sets/ref/0001_ftrl.predict

    # Test 74: cb evaluation
    )

    add_test(NAME vw_72
        COMMAND ${VW} -d train-sets/rcv1_cb_eval --cb 2 --eval
        train-sets/ref/rcv1_cb_eval.stderr

    # Test 75: Log_multi
    )

    add_test(NAME vw_73
        COMMAND ${VW} --log_multi 10 -d train-sets/multiclass
        train-sets/ref/log_multi.stderr

    # Test 76: cbify, epsilon-greedy
    )

    add_test(NAME vw_74
        COMMAND ${VW} --cbify 10 --epsilon 0.05 -d train-sets/multiclass
        train-sets/ref/cbify_epsilon.stderr

    # Test 77: cbify, tau first
    )

    add_test(NAME vw_75
        COMMAND ${VW} --cbify 10 --first 5 -d train-sets/multiclass
        train-sets/ref/cbify_first.stderr

    # Test 78: cbify, bag
    )

    add_test(NAME vw_76
        COMMAND ${VW} --cbify 10 --bag 7 -d train-sets/multiclass
        train-sets/ref/cbify_bag.stderr

    # Test 79: cbify, cover
    )

    add_test(NAME vw_77
        COMMAND ${VW} --cbify 10 --cover 3 -d train-sets/multiclass
        train-sets/ref/cbify_cover.stderr

    # Test 80: lrq empty namespace
    )

    add_test(NAME vw_78
        COMMAND ${VW} --lrq aa3 -d train-sets/0080.dat
        train-sets/ref/0080.stderr

    # Test 81: train FTRL-PiSTOL
    )

    add_test(NAME vw_79
        COMMAND ${VW} -k -d train-sets/0001.dat -f models/ftrl_pistol.model --passes 1 --pistol
        train-sets/ref/ftrl_pistol.stderr

    # Test 82: test FTRL-PiSTOL
    )

    add_test(NAME vw_80
        COMMAND ${VW} -k -t -d train-sets/0001.dat -i models/ftrl_pistol.model -p ftrl_pistol.predict
        test-sets/ref/ftrl_pistol.stderr
        pred-sets/ref/ftrl_pistol.predict

    # Test 83: check redefine functionality
    )

    add_test(NAME vw_81
        COMMAND ${VW} -k -d train-sets/0080.dat --redefine := --redefine y:=: --redefine x:=arma --ignore x -q yy
        train-sets/ref/redefine.stderr

    # Test 84: check cb_adf
    )

    add_test(NAME vw_82
        COMMAND ${VW} --cb_adf -d train-sets/cb_test.ldf --noconstant
        train-sets/ref/cb_adf.stderr

    # Test 85: check multilabel_oaa
    )

    add_test(NAME vw_83
        COMMAND ${VW} --multilabel_oaa 10 -d train-sets/multilabel -p multilabel.predict
        train-sets/ref/multilabel.stderr
        pred-sets/ref/multilabel.predict

    # Test 86: check --csoaa_rank on csoaa_ldf
    )

    add_test(NAME vw_84
        COMMAND ${VW} --csoaa_ldf multiline --csoaa_rank -d train-sets/cs_test_multilabel.ldf -p multilabel_ldf.predict --noconstant
        train-sets/ref/multilabel_ldf.stderr
        pred-sets/ref/multilabel_ldf.predict

    # Test 87: check --rank_all on csoaa_ldf
    )

    add_test(NAME vw_85
        COMMAND ${VW} --cb_adf --rank_all -d train-sets/cb_test.ldf -p cb_adf_rank.predict --noconstant
        train-sets/ref/cb_adf_rank.stderr
        pred-sets/ref/cb_adf_rank.predict

    # Test 88: named labels at training time
    )

    add_test(NAME vw_86
        COMMAND ${VW} --named_labels det,noun,verb --oaa 3 -d train-sets/test_named  -k -c --passes 10 --holdout_off -f models/test_named.model
        train-sets/ref/test_named_train.stderr

    # Test 89: named labels at prediction
    )

    add_test(NAME vw_87
        COMMAND ${VW} -i models/test_named.model -t -d train-sets/test_named -p test_named.predict
        train-sets/ref/test_named_test.stderr
        pred-sets/ref/test_named.predict

    # Test 90: named labels at training time (csoaa)
    )

    add_test(NAME vw_88
        COMMAND ${VW} --named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa  -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model
        train-sets/ref/test_named_csoaa_train.stderr

    # Test 91: named labels at prediction (csoaa)
    )

    add_test(NAME vw_89
        COMMAND ${VW} -i models/test_named_csoaa.model -t -d train-sets/test_named_csoaa -p test_named_csoaa.predict
        train-sets/ref/test_named_csoaa_test.stderr
        pred-sets/ref/test_named_csoaa.predict

    #Test 93:  check cb_adf with doubly robust option
    )

    add_test(NAME vw_90
        COMMAND ${VW} --cb_adf --rank_all -d train-sets/cb_test.ldf -p cb_adf_dr.predict --cb_type dr
        train-sets/ref/cb_adf_dr.stderr
        pred-sets/ref/cb_adf_dr.predict

    # Test 94: experience replay version of test 1
    )

    add_test(NAME vw_91
        COMMAND ${VW} -k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat 
        -c --passes 8 --invariant 
        --ngram 3 --skips 1 --holdout_off --replay_b 100
            train-sets/ref/0001-replay.stderr

    # Test 95: named labels at training time (csoaa) with experience replay
    )

    add_test(NAME vw_92
        COMMAND ${VW} --named_labels det,noun,verb --csoaa 3 
        -d train-sets/test_named_csoaa -k -c --passes 10 --holdout_off 
        -f models/test_named_csoaa.model --replay_c 100
            train-sets/ref/test_named_csoaa_train-replay.stderr

    # Test 96: backwards compatibility
    printf '3 |f a b c |e x y zn2 |f a y c |e xn' | 
        )

    add_test(NAME vw_93
        COMMAND ${VW} -i simple_model --invert_hash inv_hash.cmp && 
            tail -n +2 inv_hash.cmp
    test-sets/ref/backwards.stderr
    test-sets/ref/backwards.stdout

    # Test 97:
    )

    add_test(NAME vw_94
        COMMAND ${VW} -d train-sets/0001.dat -f models/0097.model --save_resume
            train-sets/ref/0097.stderr

    # Test 98: checking predictions as well
    )

    add_test(NAME vw_95
        COMMAND ${VW} --preserve_performance_counters -d train-sets/0001.dat -i models/0097.model -p 0098.predict
        test-sets/ref/0098.stderr
        pred-sets/ref/0098.predict

    # Test 99: checking predictions with testing
    )

    add_test(NAME vw_96
        COMMAND ${VW} -d train-sets/0001.dat -i models/0097.model -p 0099.predict
        test-sets/ref/0099.stderr
        pred-sets/ref/0099.predict

    # Test 100: action costs, no rollout
    )

    add_test(NAME vw_97
        COMMAND ${VW} -k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout none --search_task sequence_ctg --search 5 --holdout_off
        train-sets/ref/sequence_data.ctg.train.stderr

    # Test 101: active cover
    )

    add_test(NAME vw_98
        COMMAND ${VW} --loss_function logistic --binary --active_cover -d train-sets/rcv1_mini.dat -f models/active_cover.model
        train-sets/ref/active_cover.stderr

    # Test 102: active cover (predict)
    )

    add_test(NAME vw_99
        COMMAND ${VW} -i models/active_cover.model -t -d test-sets/rcv1_small_test.data -p active_cover.predict
        test-sets/ref/active_cover.stderr
        pred-sets/ref/active_cover.predict

    # Test 103: active cover oracular
    )

    add_test(NAME vw_100
        COMMAND ${VW} --loss_function logistic --binary --active_cover --oracular -d ./train-sets/rcv1_small.dat
        train-sets/ref/active_cover_oracular.stderr

    # Test 104: check cb_adf
    )

    add_test(NAME vw_101
        COMMAND ${VW} --cb_adf -d train-sets/cb_test.ldf --cb_type mtr --noconstant
        train-sets/ref/cb_adf_mtr.stderr

    # Test 105: train FTRL-Proximal early stopping
    )

    add_test(NAME vw_102
        COMMAND ${VW} -k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 3.0 --ftrl_beta 0 --l1 0.9 --cache
        train-sets/ref/0001_ftrl_holdout.stderr

    # Test 106: test FTRL-Proximal early stopping prediction
    )

    add_test(NAME vw_103
        COMMAND ${VW} -k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout.predict
        test-sets/ref/0001_ftrl_holdout_106.stderr
        pred-sets/ref/0001_ftrl_holdout.predict

    # Test 107: train FTRL-Proximal no early stopping
    )

    add_test(NAME vw_104
        COMMAND ${VW} -k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off
        train-sets/ref/0001_ftrl_holdout_off.stderr

    # Test 108: test FTRL-Proximal no early stopping
    )

    add_test(NAME vw_105
        COMMAND ${VW} -k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout_off.predict --holdout_off
        test-sets/ref/0001_ftrl_holdout_off.stderr
        pred-sets/ref/0001_ftrl_holdout_off.predict

    # Test 109: --probabilities --oaa
    )

    add_test(NAME vw_106
        COMMAND ${VW} -d train-sets/probabilities.dat --probabilities --oaa=4 --loss_function=logistic -p oaa_probabilities.predict
    train-sets/ref/oaa_probabilities.stderr
    pred-sets/ref/oaa_probabilities.predict

    # Test 110: --probabilities --csoaa_ldf=mc
    )

    add_test(NAME vw_107
        COMMAND ${VW} -d train-sets/cs_test.ldf --probabilities --csoaa_ldf=mc --loss_function=logistic -p csoaa_ldf_probabilities.predict
    train-sets/ref/csoaa_ldf_probabilities.stderr
    pred-sets/ref/csoaa_ldf_probabilities.predict

    # Test 111: Train a depenency parser with neural network and one_learner approach (lols)
    )

    add_test(NAME vw_108
        COMMAND ${VW} -k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --search_task dep_parser --search 25 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout oracle --one_learner --nn 5 --ftrl --search_history_length 3 --root_label 8
        train-sets/ref/search_dep_parser_one_learner.stderr

    # Test 112: Train a depenency parser with cost_to_go
    )

    add_test(NAME vw_109
        COMMAND ${VW} -k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --passes 6 --search_task dep_parser --search 25 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout none --holdout_off --search_history_length 3 --root_label 8 --cost_to_go
        train-sets/ref/search_dep_parser_cost_to_go.stderr

    # Test 113: Predictions with confidences
    )

    add_test(NAME vw_110
        COMMAND ${VW} --confidence -d ./train-sets/rcv1_micro.dat --initial_t 0.1 -p confidence.preds
        train-sets/ref/confidence.stderr
        pred-sets/ref/confidence.preds

    # Test 114: Over size example test
    )

    add_test(NAME vw_111
        COMMAND ${VW} -d train-sets/x.txt
        train-sets/ref/oversize.stderr

    # Test 115: Long Line test
    )

    add_test(NAME vw_112
        COMMAND ${VW} -d train-sets/long_line -c -k
        train-sets/ref/long_line.stderr

    # Test 116: MWT test
    )

    add_test(NAME vw_113
        COMMAND ${VW} -d train-sets/cb_eval --multiworld_test f -p cb_eval.preds
        train-sets/ref/cb_eval.stderr
        pred-sets/ref/cb_eval.preds

    # Test 117: Audit regressor of ftrl model (from test #107)
    )

    add_test(NAME vw_114
        COMMAND ${VW} -d train-sets/0001.dat -i models/0001_ftrl.model  --audit_regressor ftrl.audit_regr
        train-sets/ref/ftrl_audit_regr.stderr
        train-sets/ref/ftrl.audit_regr

    # Test 118: Audit regressor of csoaa model (from test #95)
    )

    add_test(NAME vw_115
        COMMAND ${VW} -d train-sets/test_named_csoaa -i models/test_named_csoaa.model --audit_regressor csoaa.audit_regr
        train-sets/ref/csoaa_audit_regr.stderr
        train-sets/ref/csoaa.audit_regr

    # Test 119: MWT learn test
    )

    add_test(NAME vw_116
        COMMAND ${VW} -d train-sets/cb_eval --multiworld_test f --learn 2 -p mwt_learn.preds
        train-sets/ref/mwt_learn.stderr
        pred-sets/ref/mwt_learn.preds

    # Test 120: MWT learn exclude test
    )

    add_test(NAME vw_117
        COMMAND ${VW} -d train-sets/cb_eval --multiworld_test f --learn 2 --exclude_eval -p mwt_learn_exclude.preds
        train-sets/ref/mwt_learn_exclude.stderr
        pred-sets/ref/mwt_learn_exclude.preds

    # Test 121: cb_explore
    )

    add_test(NAME vw_118
        COMMAND ${VW} -d train-sets/rcv1_raw_cb_small.vw --cb_explore 2 --ngram 2 --skips 4 -b 24 -l 0.25 -p rcv1_raw_cb_explore.preds
        train-sets/ref/rcv1_raw_cb_explore.stderr
        pred-sets/ref/rcv1_raw_cb_explore.preds

    # Test 122: Predictions with confidences after training
    )

    add_test(NAME vw_119
        COMMAND ${VW} --confidence --confidence_after_training --initial_t 0.1 -d ./train-sets/rcv1_small.dat -p confidence_after_training.preds
        train-sets/ref/confidence_after_training.stderr
        pred-sets/ref/confidence_after_training.preds

    # Test 123: cb_eval save/load #1
    )

    add_test(NAME vw_120
        COMMAND ${VW} -d train-sets/cb_eval1 --multiworld_test f -f mwt.model -p cb_eval1.preds
        train-sets/ref/cb_eval1.stderr
        pred-sets/ref/cb_eval1.preds

    # Test 124: cb_eval save/load #2
    )

    add_test(NAME vw_121
        COMMAND ${VW} -d train-sets/cb_eval2 -i mwt.model -p cb_eval2.preds
        train-sets/ref/cb_eval2.stderr
        pred-sets/ref/cb_eval2.preds

    # Test 125: arc-eager trasition-based dependency parser
    )

    add_test(NAME vw_122
        COMMAND ${VW} -k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --search_task dep_parser --search 26 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout oracle --one_learner --search_history_length 3 --root_label 8 --transition_system 2 --passes 8
        train-sets/ref/search_dep_parser_arceager.stderr

    # Test 126: recall tree hello world
    )

    add_test(NAME vw_123
        COMMAND ${VW} --quiet -d train-sets/gauss1k.dat.gz -f models/recall_tree_g100.model --recall_tree 100 -b 20 --loss_function logistic

    # Test 127: recall_tree hello world predict-from-saved-model
    )

    add_test(NAME vw_124
        COMMAND ${VW} -t -d train-sets/gauss1k.dat.gz -i models/recall_tree_g100.model
        train-sets/ref/recall_tree_gauss1k.stderr
        train-sets/ref/recall_tree_gauss1k.stdout

    # Test 128: cb_explore_adf with epsilon-greedy exploration
    )

    add_test(NAME vw_125
        COMMAND ${VW} --cb_explore_adf --epsilon 0.1 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_epsilon.predict
        train-sets/ref/cbe_adf_epsilon.stderr
        pred-sets/ref/cbe_adf_epsilon.predict

    # Test 129: cb_explore_adf with softmax exploration
    )

    add_test(NAME vw_126
        COMMAND ${VW} --cb_explore_adf --softmax --lambda 1 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_softmax.predict
        train-sets/ref/cbe_adf_softmax.stderr
        pred-sets/ref/cbe_adf_softmax.predict

    # Test 130: cb_explore_adf with bagging exploration
    )

    add_test(NAME vw_127
        COMMAND ${VW} --cb_explore_adf --bag 3 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_bag.predict
        train-sets/ref/cbe_adf_bag.stderr
        pred-sets/ref/cbe_adf_bag.predict

    # Test 131: cb_explore_adf with explore-first exploration
    )

    add_test(NAME vw_128
        COMMAND ${VW} --cb_explore_adf --first 2 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_first.predict
        train-sets/ref/cbe_adf_first.stderr
        pred-sets/ref/cbe_adf_first.predict

    # Test 132: train a poisson model
    )

    add_test(NAME vw_129
        COMMAND ${VW} --quiet -d train-sets/poisson.dat -f models/poisson.model --loss_function poisson --link poisson -b 2 -p poisson.train.predict
        train-sets/ref/poisson.train.stderr
        pred-sets/ref/poisson.train.predict

    # Test 133: train a poisson model without invariant updates
    )

    add_test(NAME vw_130
        COMMAND ${VW} --quiet -d train-sets/poisson.dat -f models/poisson.normalized.model --normalized --loss_function poisson --link poisson -b 2 -l 0.1 -p poisson.train.normalized.predict
        train-sets/ref/poisson.train.normalized.stderr
        pred-sets/ref/poisson.train.normalized.predict

    # Test 134: second order online learning
    )

    add_test(NAME vw_131
        COMMAND ${VW} --OjaNewton -d train-sets/0001.dat -f models/second_order.model -p second_order.predict
        train-sets/ref/second_order.stderr
        pred-sets/ref/second_order.predict

    # Test 135: cb explore adf
    )

    add_test(NAME vw_132
        COMMAND ${VW} -d train-sets/cb_adf_crash_1.data -f models/cb_adf_crash.model --cb_explore_adf --epsilon 0.05
        train-sets/ref/cb_adf_crash1.stderr

    # Test 136: cb explore adf predict
    )

    add_test(NAME vw_133
        COMMAND ${VW} -d train-sets/cb_adf_crash_2.data -i models/cb_adf_crash.model -t
        train-sets/ref/cb_adf_crash2.stderr

    # Test 137: Fix for regression introduced by badeedb.
    # Ensure audit output continues to work correctly in the presence of anon features.
    # Github issue 1038 (https://github.com/JohnLangford/vowpal_wabbit/issues/1038)
    )

    add_test(NAME vw_134
        COMMAND ${VW} --audit -d train-sets/audit.dat --noconstant
        train-sets/ref/audit.stderr
        train-sets/ref/audit.stdout

    # Test 138: cb_explore_adf with cover exploration
    )

    add_test(NAME vw_135
        COMMAND ${VW} --cb_explore_adf --cover 3 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_cover.predict
        train-sets/ref/cbe_adf_cover.stderr
        pred-sets/ref/cbe_adf_cover.predict

    # Test 139: cb_explore_adf with cover exploration + double robust
    )

    add_test(NAME vw_136
        COMMAND ${VW} --cb_explore_adf --cover 3 --cb_type dr -d train-sets/cb_test.ldf --noconstant -p cbe_adf_cover_dr.predict
        train-sets/ref/cbe_adf_cover_dr.stderr
        pred-sets/ref/cbe_adf_cover_dr.predict

    # Test 140: marginal features
    )

    add_test(NAME vw_137
        COMMAND ${VW} --marginal f  -d train-sets/marginal_features --noconstant --initial_numerator 0.5 --initial_denominator 1.0 --decay 0.001 --holdout_off -c -k --passes 100 -f marginal_model
        train-sets/ref/marginal.stderr

    # Test 141: marginal features test
    )

    add_test(NAME vw_138
        COMMAND ${VW} -i marginal_model  -d train-sets/marginal_features --noconstant -t
        train-sets/ref/marginal_test.stderr

    # Test 142: Evaluate exploration on contextal bandit data
    )

    add_test(NAME vw_139
        COMMAND ${VW} --explore_eval --epsilon 0.2 -d train-sets/cb_test.ldf --noconstant -p explore_eval.predict
        train-sets/ref/explore_eval.stderr
        pred-sets/ref/explore_eval.predict

    # Test 143: Test 1 using JSON
    )

    add_test(NAME vw_140
        COMMAND ${VW} -k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.json --json 
        -c --passes 8 --invariant 
        --ngram 3 --skips 1 --holdout_off
            train-sets/ref/0001.json.stderr

    # Test 144: cb_explore_adf with cover exploration + double robust
    )

    add_test(NAME vw_141
        COMMAND ${VW} --cb_explore_adf --cover 3 --cb_type dr -d train-sets/cb_test.json --json --noconstant -p cbe_adf_cover_dr.predict
        train-sets/ref/cbe_adf_cover_dr.json.stderr
        pred-sets/ref/cbe_adf_cover_dr.predict

    # Test 145: mix labeled and unlabeled examples with --bootstrap bug:
    # https://github.com/JohnLangford/vowpal_wabbit/issues/1111
    )

    add_test(NAME vw_142
        COMMAND ${VW} --bootstrap 2 -d train-sets/labeled-unlabeled-mix.dat
        train-sets/ref/labeled-unlabeled-mix.stderr

    # Test 146: cb_explore_adf with cover exploration + double robust (using more than 256 examples)
    )

    add_test(NAME vw_143
        COMMAND ${VW} --cb_explore_adf --cover 3 --cb_type dr -d train-sets/cb_test256.json --json --noconstant -p cbe_adf_cover_dr256.predict
        train-sets/ref/cbe_adf_cover_dr256.json.stderr
        pred-sets/ref/cbe_adf_cover_dr256.predict

    # Test 147: --scores --oaa
    )

    add_test(NAME vw_144
        COMMAND ${VW} -d train-sets/probabilities.dat --scores --oaa=4 -p oaa_scores.predict
    train-sets/ref/oaa_scores.stderr
    pred-sets/ref/oaa_scores.predict

    # Test 148:  check cb_adf with direct method option
    )

    add_test(NAME vw_145
        COMMAND ${VW} --cb_adf -d train-sets/cb_test.ldf -p cb_adf_dm.predict --cb_type dm
        train-sets/ref/cb_adf_dm.stderr
        pred-sets/ref/cb_adf_dm.predict


    # Test 150:  Test --sparse_weights with 148
    )

    add_test(NAME vw_146
        COMMAND ${VW} --cb_adf -d train-sets/cb_test.ldf -p cb_adf_dm.predict --cb_type dm --sparse_weights
        train-sets/ref/sparse.stderr

    # Test 151: lrqfa
    )

    add_test(NAME vw_147
        COMMAND ${VW} --lrqfa aa3 -d train-sets/0080.dat
        train-sets/ref/0151.stderr


    # Test 153: marginal features
    )

    add_test(NAME vw_148
        COMMAND ${VW} --marginal f  -d train-sets/marginal_features --noconstant --initial_numerator 0.5 --initial_denominator 1.0 --decay 0.001 --holdout_off -c -k --passes 100  --compete
        train-sets/ref/marginal_compete.stderr

    # Test 154: ignore linear
    )

    add_test(NAME vw_149
        COMMAND ${VW} -k --cache_file ignore_linear.cache --passes 10000 --holdout_off -d train-sets/0154.dat --noconstant --ignore_linear x -q xx
        train-sets/ref/ignore_linear.stderr

    # Test 155: checking audit_regressor with --save_resume model
    )

    add_test(NAME vw_150
        COMMAND ${VW} -d train-sets/0001.dat -i models/0097.model --save_resume --audit_regressor 0097.audit_regr
        train-sets/ref/0097.audit_regr.stderr
        train-sets/ref/0097.audit_regr


    # Test 157: save_resume without --preserve_performce_counters does not alter performance counters over multiple passes
    )

    add_test(NAME vw_151
        COMMAND ${VW} -d train-sets/0001.dat -f models/sr.model  --passes 2 -c -k  -P 50 --save_resume
        train-sets/ref/157.stderr

    # Test 158: test decision service json parsing
    )

    add_test(NAME vw_152
        COMMAND ${VW} -d train-sets/decisionservice.json --dsjson --cb_explore_adf --epsilon 0.2 --quadratic GT
        train-sets/ref/decisionservice.stderr

    # Test 159: test --bootstrap & --binary interaction
    )

    add_test(NAME vw_153
        COMMAND ${VW} -d train-sets/rcv1_mini.dat --bootstrap 5 --binary -c -k --passes 2
        train-sets/ref/bootstrap_and_binary.stderr

    # Test 160: test --bootstrap & --oaa interaction
    # (Also adds -q :: and -P1 to get & verify perfect predictions in 2nd pass)
    )

    add_test(NAME vw_154
        COMMAND ${VW} -d train-sets/multiclass --bootstrap 4 --oaa 10 -q :: --leave_duplicate_interactions  -c -k --passes 2 --holdout_off -P1
        train-sets/ref/bootstrap_and_oaa.stderr

    # Test 161: --classweight
    )

    add_test(NAME vw_155
        COMMAND ${VW} -d train-sets/0001.dat --classweight 1:2,0:3.1,-1:5
        train-sets/ref/classweight.stderr

    # Test 162: --classweight with multiclass
    )

    add_test(NAME vw_156
        COMMAND ${VW} --oaa 10 -d train-sets/multiclass --classweight 4:0,7:0.1,2:10,10:3
        train-sets/ref/classweight_multiclass.stderr

    # Test 163: --classweight with multiclass
    )

    add_test(NAME vw_157
        COMMAND ${VW} --recall_tree 10 -d train-sets/multiclass --classweight 4:0,7:0.1,2:10,10:3
        train-sets/ref/classweight_recall_tree.stderr

    # Test 164: cs_active low mellowness
    )

    add_test(NAME vw_158
        COMMAND ${VW} --cs_active 3 -d ../test/train-sets/cs_test --cost_max 2 --mellowness 0.01 --simulation --adax
        train-sets/ref/cs_active_0.01.stderr

    # Test 165: cs_active high mellowness
    )

    add_test(NAME vw_159
        COMMAND ${VW} --cs_active 3 -d ../test/train-sets/cs_test --cost_max 2 --mellowness 1.0 --simulation --adax
        train-sets/ref/cs_active_1.0.stderr

    # Test 166: hash_seed train
    )

    add_test(NAME vw_160
        COMMAND ${VW} --hash_seed 5 -d train-sets/rcv1_mini.dat --holdout_off --passes 2 -f hash_seed5.model -c -k --ngram 2 -q ::
        train-sets/ref/hash_seed_train.stderr

    # Test 167: hash_seed test
    )

    add_test(NAME vw_161
        COMMAND ${VW} -d train-sets/rcv1_mini.dat -i hash_seed5.model -t
        train-sets/ref/hash_seed_test.stderr
    )
endif()