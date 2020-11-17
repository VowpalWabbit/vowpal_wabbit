( ../build/vowpalwabbit/vw -k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat      -f models/0001_1.model -c --passes 8 --invariant      --ngram 3 --skips 1 --holdout_off ) >outputs/1_Test.stdout.track 2>outputs/1_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -d train-sets/0001.dat -i models/0001_1.model -p 0001.predict --invariant ) >outputs/2_Test.stdout.track 2>outputs/2_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0002.dat -f models/0002.model --invariant ) >outputs/3_Test.stdout.track 2>outputs/3_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0002.dat -f models/0002.model --invariant ) >outputs/4_Test.stdout.track 2>outputs/4_Test.stderr.track
( ../build/vowpalwabbit/vw -k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model -d train-sets/0002.dat ) >outputs/5_Test.stdout.track 2>outputs/5_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -i models/0002.model -d train-sets/0002.dat -p 0002b.predict ) >outputs/6_Test.stdout.track 2>outputs/6_Test.stderr.track
( ../build/vowpalwabbit/vw -k --power_t 0.45 -f models/0002c.model -d train-sets/0002.dat ) >outputs/7_Test.stdout.track 2>outputs/7_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -i models/0002c.model -d train-sets/0002.dat -p 0002c.predict ) >outputs/8_Test.stdout.track 2>outputs/8_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/cs_test.ldf -p cs_test.ldf.csoaa.predict --passes 10 --invariant --csoaa_ldf multiline --holdout_off --noconstant ) >outputs/9_Test.stdout.track 2>outputs/9_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/cs_test.ldf -p cs_test.ldf.wap.predict --passes 10 --invariant --wap_ldf multiline --holdout_off --noconstant ) >outputs/10_Test.stdout.track 2>outputs/10_Test.stderr.track
( ../build/vowpalwabbit/vw -k --oaa 10 -c --passes 10 -d train-sets/multiclass --holdout_off ) >outputs/11_Test.stdout.track 2>outputs/11_Test.stderr.track
( ../build/vowpalwabbit/vw -k --ect 10 --error 3 -c --passes 10 --invariant -d train-sets/multiclass --holdout_off ) >outputs/12_Test.stdout.track 2>outputs/12_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/wsj_small.dat.gz --passes 6      --search_task sequence --search 45 --search_alpha 1e-6      --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1      --holdout_off ) >outputs/13_Test.stdout.track 2>outputs/13_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/wsj_small.dat.gz --passes 6      --search_task sequence --search 45 --search_alpha 1e-6      --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1      --holdout_off --search_passes_per_policy 3 --search_interpolation policy ) >outputs/14_Test.stdout.track 2>outputs/14_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/zero.dat --loss_function=squared -b 20 --bfgs --mem 7 --passes 5 --l2 1.0 --holdout_off ) >outputs/15_Test.stdout.track 2>outputs/15_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/rcv1_small.dat --loss_function=logistic --bfgs --mem 7 --passes 20 --termination 0.001 --l2 1.0 --holdout_off ) >outputs/16_Test.stdout.track 2>outputs/16_Test.stderr.track
( ../build/vowpalwabbit/vw -k --lda 100 --lda_alpha 0.01 --lda_rho 0.01 --lda_D 1000 -l 1 -b 13 --minibatch 128 -d train-sets/wiki256.dat ) >outputs/17_Test.stdout.track 2>outputs/17_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/seq_small --passes 12 --invariant --search 4 --search_task sequence --holdout_off ) >outputs/18_Test.stdout.track 2>outputs/18_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/3parity --hash all --passes 3000 -b 16 --nn 2 -l 10 --invariant -f models/0021.model --random_seed 19 --quiet --holdout_off ) >outputs/19_Test.stdout.track 2>outputs/19_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/3parity -t -i models/0021.model -p 0022.predict ) >outputs/20_Test.stdout.track 2>outputs/20_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -f models/xxor.model -d train-sets/xxor.dat --cubic abc --passes 100 --holdout_off --progress 1.33333 ) >outputs/21_Test.stdout.track 2>outputs/21_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/ml100k_small_train -b 16 -q ui --rank 10      --l2 2e-6 --learning_rate 0.05 --passes 2      --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg      -c --loss_function classic --holdout_off ) >outputs/22_Test.stdout.track 2>outputs/22_Test.stderr.track
( ../build/vowpalwabbit/vw -i models/movielens.reg -t -d test-sets/ml100k_small_test ) >outputs/23_Test.stdout.track 2>outputs/23_Test.stderr.track
( ../build/vowpalwabbit/vw -k --active --simulation --mellowness 0.000001 -d train-sets/rcv1_small.dat -l 10 --initial_t 10 --random_seed 3 ) >outputs/24_Test.stdout.track 2>outputs/24_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0002.dat -f models/bs.reg.model --bootstrap 4 -p bs.reg.predict ) >outputs/25_Test.stdout.track 2>outputs/25_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0002.dat -i models/bs.reg.model -p bs.prreg.predict -t ) >outputs/26_Test.stdout.track 2>outputs/26_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict ) >outputs/27_Test.stdout.track 2>outputs/27_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -i models/bs.vote.model -p bs.prvote.predict -t ) >outputs/28_Test.stdout.track 2>outputs/28_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/affix_test.dat -k -c --passes 10 --holdout_off --affix -2 ) >outputs/29_Test.stdout.track 2>outputs/29_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -f models/mask.model --invert_hash mask.predict --l1 0.01 ) >outputs/30_Test.stdout.track 2>outputs/30_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat --invert_hash remask.predict --feature_mask models/mask.model -f models/remask.model ) >outputs/31_Test.stdout.track 2>outputs/31_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat --feature_mask models/mask.model -i models/remask.model ) >outputs/32_Test.stdout.track 2>outputs/32_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/topk. ) >outputs/33_Test.stdout.track 2>outputs/33_Test.stderr.track
( ../build/vowpalwabbit/vw -P 1 -d train-sets/topk. ) >outputs/34_Test.stdout.track 2>outputs/34_Test.stderr.track
( ../build/vowpalwabbit/vw -k --passes 100 -c --holdout_off --constant 1000 -d train-sets/big-constant.dat ) >outputs/35_Test.stdout.track 2>outputs/35_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat --progress 10 ) >outputs/36_Test.stdout.track 2>outputs/36_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat -P 0.5 ) >outputs/37_Test.stdout.track 2>outputs/37_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat --nn 1 ) >outputs/38_Test.stdout.track 2>outputs/38_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_raw_cb_small. ) >outputs/39_Test.stdout.track 2>outputs/39_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_raw_cb_small. ) >outputs/40_Test.stdout.track 2>outputs/40_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_raw_cb_small. ) >outputs/41_Test.stdout.track 2>outputs/41_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/lda-2pass-hang.dat --lda 10 -c --passes 2 --holdout_off ) >outputs/42_Test.stdout.track 2>outputs/42_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequence --search 5 --holdout_off -f models/sequence_data.model ) >outputs/43_Test.stdout.track 2>outputs/43_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.test.predict ) >outputs/44_Test.stdout.track 2>outputs/44_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/seq_small2 --passes 4 --search 4 --search_task sequence --holdout_off ) >outputs/45_Test.stdout.track 2>outputs/45_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/sequence_data --passes 20      --search_rollout ref --search_alpha 1e-8      --search_task sequence_demoldf --csoaa_ldf m --search 5      --holdout_off -f models/sequence_data.ldf.model --noconstant ) >outputs/46_Test.stdout.track 2>outputs/46_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.test.predict --noconstant ) >outputs/47_Test.stdout.track 2>outputs/47_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/sequencespan_data --passes 20 --invariant      --search_rollout none --search_task sequencespan --search 7      --holdout_off -f models/sequencespan_data.model ) >outputs/48_Test.stdout.track 2>outputs/48_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/sequencespan_data -t -i models/sequencespan_data.model -p sequencespan_data.nonldf.test.predict ) >outputs/49_Test.stdout.track 2>outputs/49_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/sequencespan_data --passes 20 --invariant      --search_rollout ref --search_alpha 1e-8 --search_task sequencespan      --search_span_bilou --search 7 --holdout_off      -f models/sequencespan_data.model ) >outputs/50_Test.stdout.track 2>outputs/50_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/sequencespan_data -t --search_span_bilou -i models/sequencespan_data.model -p sequencespan_data.nonldf-bilou.test.predict ) >outputs/51_Test.stdout.track 2>outputs/51_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/argmax_data -k -c --passes 20 --search_rollout ref --search_alpha 1e-8 --search_task argmax --search 2 --holdout_off ) >outputs/52_Test.stdout.track 2>outputs/52_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c --passes 2 -d train-sets/0001.dat ) >outputs/53_Test.stdout.track 2>outputs/53_Test.stderr.track
( ../build/vowpalwabbit/vw --stage_poly --sched_exponent 0.25 --batch_sz 1000 --batch_sz_no_doubling -d train-sets/rcv1_small.dat -p stage_poly.s025.predict --quiet ) >outputs/54_Test.stdout.track 2>outputs/54_Test.stderr.track
( ../build/vowpalwabbit/vw --stage_poly --sched_exponent 1.0 --batch_sz 1000 --batch_sz_no_doubling -d train-sets/rcv1_small.dat --quiet ) >outputs/55_Test.stdout.track 2>outputs/55_Test.stderr.track
( ../build/vowpalwabbit/vw --stage_poly --sched_exponent 0.25 --batch_sz 1000 -d train-sets/rcv1_small.dat -p stage_poly.s025.doubling.predict --quiet ) >outputs/56_Test.stdout.track 2>outputs/56_Test.stderr.track
( ../build/vowpalwabbit/vw --stage_poly --sched_exponent 1.0 --batch_sz 1000 -d train-sets/rcv1_small.dat -p stage_poly.s100.doubling.predict --quiet ) >outputs/57_Test.stdout.track 2>outputs/57_Test.stderr.track
( ../build/vowpalwabbit/vw -c -k -d train-sets/library_train -f models/library_train.w -q st --passes 100 --hash all --noconstant --csoaa_ldf m --holdout_off ) >outputs/58_Test.stdout.track 2>outputs/58_Test.stderr.track
( ../build/vowpalwabbit/vw  --dsjson --cb_adf -d train-sets/no_shared_features.json ) >outputs/59_Test.stdout.track 2>outputs/59_Test.stderr.track
( echo "" | ../build/vowpalwabbit/vwvw  >/dev/null 2>empty-set.stderr ) >outputs/60_Test.stdout.track 2>outputs/60_Test.stderr.track
( ./daemon-test.shvw  >vw-daemon.stdout 2>/dev/null ) >outputs/61_Test.stdout.track 2>outputs/61_Test.stderr.track
( ../build/vowpalwabbit/vw --ksvm --l2 1 --reprocess 5 -b 18 -p ksvm_train.linear.predict -d train-sets/rcv1_smaller.dat ) >outputs/62_Test.stdout.track 2>outputs/62_Test.stderr.track
( ../build/vowpalwabbit/vw --ksvm --l2 1 --reprocess 5 -b 18 --kernel poly -p ksvm_train.poly.predict -d train-sets/rcv1_smaller.dat ) >outputs/63_Test.stdout.track 2>outputs/63_Test.stderr.track
( ../build/vowpalwabbit/vw --ksvm --l2 1 --reprocess 5 -b 18 --kernel rbf -p ksvm_train.rbf.predict -d train-sets/rcv1_smaller.dat ) >outputs/64_Test.stdout.track 2>outputs/64_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/er_small. ) >outputs/65_Test.stdout.track 2>outputs/65_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/wsj_small.dparser.vw.gz --passes 6 --search_task dep_parser --search 12  --search_alpha 1e-4 --search_rollout oracle --holdout_off ) >outputs/66_Test.stdout.track 2>outputs/66_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/dictionary_test.dat --binary --ignore w --holdout_off --passes 32 --dictionary w ) >outputs/67_Test.stdout.track 2>outputs/67_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/multiclass.sch --passes 20 --search_task multiclasstask --search 10 --search_alpha 1e-4 --holdout_off ) >outputs/68_Test.stdout.track 2>outputs/68_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10 ) >outputs/69_Test.stdout.track 2>outputs/69_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10 --noconstant ) >outputs/70_Test.stdout.track 2>outputs/70_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0002.dat --autolink 1 --examples 100 -p 0002.autolink.predict ) >outputs/71_Test.stdout.track 2>outputs/71_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 1 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 ) >outputs/72_Test.stdout.track 2>outputs/72_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl.predict ) >outputs/73_Test.stdout.track 2>outputs/73_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_cb_eval --cb 2 --eval ) >outputs/74_Test.stdout.track 2>outputs/74_Test.stderr.track
( ../build/vowpalwabbit/vw --log_multi 10 -d train-sets/multiclass ) >outputs/75_Test.stdout.track 2>outputs/75_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 10 --epsilon 0.05 -d train-sets/multiclass ) >outputs/76_Test.stdout.track 2>outputs/76_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 10 --first 5 -d train-sets/multiclass ) >outputs/77_Test.stdout.track 2>outputs/77_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 10 --bag 7 -d train-sets/multiclass ) >outputs/78_Test.stdout.track 2>outputs/78_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 10 --cover 3 -d train-sets/multiclass ) >outputs/79_Test.stdout.track 2>outputs/79_Test.stderr.track
( ../build/vowpalwabbit/vw --lrq aa3 -d train-sets/0080.dat ) >outputs/80_Test.stdout.track 2>outputs/80_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat -f models/ftrl_pistol.model --passes 1 --pistol ) >outputs/81_Test.stdout.track 2>outputs/81_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -d train-sets/0001.dat -i models/ftrl_pistol.model -p ftrl_pistol.predict ) >outputs/82_Test.stdout.track 2>outputs/82_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0080.dat --redefine  ) >outputs/83_Test.stdout.track 2>outputs/83_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_adf -d train-sets/cb_test.ldf --noconstant ) >outputs/84_Test.stdout.track 2>outputs/84_Test.stderr.track
( ../build/vowpalwabbit/vw --multilabel_oaa 10 -d train-sets/multilabel -p multilabel.predict ) >outputs/85_Test.stdout.track 2>outputs/85_Test.stderr.track
( ../build/vowpalwabbit/vw --csoaa_ldf multiline --csoaa_rank -d train-sets/cs_test_multilabel.ldf -p multilabel_ldf.predict --noconstant ) >outputs/86_Test.stdout.track 2>outputs/86_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_adf --rank_all -d train-sets/cb_test.ldf -p cb_adf_rank.predict --noconstant ) >outputs/87_Test.stdout.track 2>outputs/87_Test.stderr.track
( ../build/vowpalwabbit/vw --named_labels det,noun,verb --oaa 3 -d train-sets/test_named  -k -c --passes 10 --holdout_off -f models/test_named.model ) >outputs/88_Test.stdout.track 2>outputs/88_Test.stderr.track
( ../build/vowpalwabbit/vw -i models/test_named.model -t -d train-sets/test_named -p test_named.predict ) >outputs/89_Test.stdout.track 2>outputs/89_Test.stderr.track
( ../build/vowpalwabbit/vw --named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa  -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model ) >outputs/90_Test.stdout.track 2>outputs/90_Test.stderr.track
( ../build/vowpalwabbit/vw -i models/test_named_csoaa.model -t -d train-sets/test_named_csoaa -p test_named_csoaa.predict ) >outputs/91_Test.stdout.track 2>outputs/91_Test.stderr.track
( printf '3 |f a b c |e x y z\n2 |f a y c |e x\n' |      ../build/vowpalwabbit/vw --oaa 3 -q  ) >outputs/92_Test.stdout.track 2>outputs/92_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_adf --rank_all -d train-sets/cb_test.ldf -p cb_adf_dr.predict --cb_type dr ) >outputs/93_Test.stdout.track 2>outputs/93_Test.stderr.track
( ../build/vowpalwabbit/vw -k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat      -c --passes 8 --invariant      --ngram 3 --skips 1 --holdout_off --replay_b 100 ) >outputs/94_Test.stdout.track 2>outputs/94_Test.stderr.track
( ../build/vowpalwabbit/vw --named_labels det,noun,verb --csoaa 3      -d train-sets/test_named_csoaa -k -c --passes 10 --holdout_off      -f models/test_named_csoaa.model --replay_c 100 ) >outputs/95_Test.stdout.track 2>outputs/95_Test.stderr.track
( printf '3 |f a b c |e x y z\n2 |f a y c |e x\n' |      ../build/vowpalwabbit/vw -i simple_model --invert_hash inv_hash.cmp &&          tail -n +2 inv_hash.cmp ) >outputs/96_Test.stdout.track 2>outputs/96_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -f models/0097.model --save_resume ) >outputs/97_Test.stdout.track 2>outputs/97_Test.stderr.track
( ../build/vowpalwabbit/vw --preserve_performance_counters -d train-sets/0001.dat -i models/0097.model -p 0098.predict ) >outputs/98_Test.stdout.track 2>outputs/98_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -i models/0097.model -p 0099.predict ) >outputs/99_Test.stdout.track 2>outputs/99_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout none --search_task sequence_ctg --search 5 --holdout_off ) >outputs/100_Test.stdout.track 2>outputs/100_Test.stderr.track
( ../build/vowpalwabbit/vw --loss_function logistic --binary --active_cover -d train-sets/rcv1_mini.dat -f models/active_cover.model ) >outputs/101_Test.stdout.track 2>outputs/101_Test.stderr.track
( ../build/vowpalwabbit/vw -i models/active_cover.model -t -d test-sets/rcv1_small_test.data -p active_cover.predict ) >outputs/102_Test.stdout.track 2>outputs/102_Test.stderr.track
( ../build/vowpalwabbit/vw --loss_function logistic --binary --active_cover --oracular -d ./train-sets/rcv1_small.dat ) >outputs/103_Test.stdout.track 2>outputs/103_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_adf -d train-sets/cb_test.ldf --cb_type mtr --noconstant ) >outputs/104_Test.stdout.track 2>outputs/104_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 3.0 --ftrl_beta 0 --l1 0.9 --cache ) >outputs/105_Test.stdout.track 2>outputs/105_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout.predict ) >outputs/106_Test.stdout.track 2>outputs/106_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off ) >outputs/107_Test.stdout.track 2>outputs/107_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout_off.predict --holdout_off ) >outputs/108_Test.stdout.track 2>outputs/108_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/probabilities.dat --probabilities --oaa=4 --loss_function=logistic -p oaa_probabilities.predict ) >outputs/109_Test.stdout.track 2>outputs/109_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cs_test.ldf --probabilities --csoaa_ldf=mc --loss_function=logistic -p csoaa_ldf_probabilities.predict ) >outputs/110_Test.stdout.track 2>outputs/110_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --search_task dep_parser --search 25 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout oracle --one_learner --nn 5 --ftrl --search_history_length 3 --root_label 8 ) >outputs/111_Test.stdout.track 2>outputs/111_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --passes 6 --search_task dep_parser --search 25 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout none --holdout_off --search_history_length 3 --root_label 8 --cost_to_go ) >outputs/112_Test.stdout.track 2>outputs/112_Test.stderr.track
( ../build/vowpalwabbit/vw --confidence -d ./train-sets/rcv1_micro.dat --initial_t 0.1 -p confidence.preds ) >outputs/113_Test.stdout.track 2>outputs/113_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/x.txt ) >outputs/114_Test.stdout.track 2>outputs/114_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/long_line -c -k ) >outputs/115_Test.stdout.track 2>outputs/115_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cb_eval --multiworld_test f -p cb_eval.preds ) >outputs/116_Test.stdout.track 2>outputs/116_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -i models/0001_ftrl.model  --audit_regressor ftrl.audit_regr ) >outputs/117_Test.stdout.track 2>outputs/117_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/test_named_csoaa -i models/test_named_csoaa.model --audit_regressor csoaa.audit_regr ) >outputs/118_Test.stdout.track 2>outputs/118_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cb_eval --multiworld_test f --learn 2 -p mwt_learn.preds ) >outputs/119_Test.stdout.track 2>outputs/119_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cb_eval --multiworld_test f --learn 2 --exclude_eval -p mwt_learn_exclude.preds ) >outputs/120_Test.stdout.track 2>outputs/120_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_raw_cb_small. ) >outputs/121_Test.stdout.track 2>outputs/121_Test.stderr.track
( ../build/vowpalwabbit/vw --confidence --confidence_after_training --initial_t 0.1 -d ./train-sets/rcv1_small.dat -p confidence_after_training.preds ) >outputs/122_Test.stdout.track 2>outputs/122_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cb_eval1 --multiworld_test f -f mwt.model -p cb_eval1.preds ) >outputs/123_Test.stdout.track 2>outputs/123_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cb_eval2 -i mwt.model -p cb_eval2.preds ) >outputs/124_Test.stdout.track 2>outputs/124_Test.stderr.track
( ../build/vowpalwabbit/vw -k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --search_task dep_parser --search 26 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout oracle --one_learner --search_history_length 3 --root_label 8 --transition_system 2 --passes 8 ) >outputs/125_Test.stdout.track 2>outputs/125_Test.stderr.track
( ../build/vowpalwabbit/vw --quiet -d train-sets/gauss1k.dat.gz -f models/recall_tree_g100.model --recall_tree 100 -b 20 --loss_function logistic ) >outputs/126_Test.stdout.track 2>outputs/126_Test.stderr.track
( ../build/vowpalwabbit/vw -t -d train-sets/gauss1k.dat.gz -i models/recall_tree_g100.model ) >outputs/127_Test.stdout.track 2>outputs/127_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --epsilon 0.1 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_epsilon.predict ) >outputs/128_Test.stdout.track 2>outputs/128_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --softmax --lambda 1 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_softmax.predict ) >outputs/129_Test.stdout.track 2>outputs/129_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --bag 3 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_bag.predict ) >outputs/130_Test.stdout.track 2>outputs/130_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --first 2 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_first.predict ) >outputs/131_Test.stdout.track 2>outputs/131_Test.stderr.track
( ../build/vowpalwabbit/vw --quiet -d train-sets/poisson.dat -f models/poisson.model --loss_function poisson --link poisson -b 2 -p poisson.train.predict ) >outputs/132_Test.stdout.track 2>outputs/132_Test.stderr.track
( ../build/vowpalwabbit/vw --quiet -d train-sets/poisson.dat -f models/poisson.normalized.model --normalized --loss_function poisson --link poisson -b 2 -l 0.1 -p poisson.train.normalized.predict ) >outputs/133_Test.stdout.track 2>outputs/133_Test.stderr.track
( ../build/vowpalwabbit/vw --OjaNewton -d train-sets/0001.dat -f models/second_order.model -p second_order.predict ) >outputs/134_Test.stdout.track 2>outputs/134_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cb_adf_crash_1.data -f models/cb_adf_crash.model --cb_explore_adf --epsilon 0.05 ) >outputs/135_Test.stdout.track 2>outputs/135_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cb_adf_crash_2.data -i models/cb_adf_crash.model -t ) >outputs/136_Test.stdout.track 2>outputs/136_Test.stderr.track
( ../build/vowpalwabbit/vw --audit -d train-sets/audit.dat --noconstant ) >outputs/137_Test.stdout.track 2>outputs/137_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --cover 3 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_cover.predict ) >outputs/138_Test.stdout.track 2>outputs/138_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --cover 3 --cb_type dr -d train-sets/cb_test.ldf --noconstant -p cbe_adf_cover_dr.predict ) >outputs/139_Test.stdout.track 2>outputs/139_Test.stderr.track
( ../build/vowpalwabbit/vw --marginal f  -d train-sets/marginal_features --noconstant --initial_numerator 0.5 --initial_denominator 1.0 --decay 0.001 --holdout_off -c -k --passes 100 -f marginal_model ) >outputs/140_Test.stdout.track 2>outputs/140_Test.stderr.track
( ../build/vowpalwabbit/vw -i marginal_model  -d train-sets/marginal_features --noconstant -t ) >outputs/141_Test.stdout.track 2>outputs/141_Test.stderr.track
( ../build/vowpalwabbit/vw --explore_eval --epsilon 0.2 -d train-sets/cb_test.ldf --noconstant -p explore_eval.predict ) >outputs/142_Test.stdout.track 2>outputs/142_Test.stderr.track
( ../build/vowpalwabbit/vw -k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.json --json      -c --passes 8 --invariant      --ngram 3 --skips 1 --holdout_off ) >outputs/143_Test.stdout.track 2>outputs/143_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --cover 3 --cb_type dr -d train-sets/cb_test.json --json --noconstant -p cbe_adf_cover_dr.predict ) >outputs/144_Test.stdout.track 2>outputs/144_Test.stderr.track
( ../build/vowpalwabbit/vw --bootstrap 2 -d train-sets/labeled-unlabeled-mix.dat ) >outputs/145_Test.stdout.track 2>outputs/145_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --cover 3 --cb_type dr -d train-sets/cb_test256.json --json --noconstant -p cbe_adf_cover_dr256.predict ) >outputs/146_Test.stdout.track 2>outputs/146_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/probabilities.dat --scores --oaa=4 -p oaa_scores.predict ) >outputs/147_Test.stdout.track 2>outputs/147_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_adf -d train-sets/cb_test.ldf -p cb_adf_dm.predict --cb_type dm ) >outputs/148_Test.stdout.track 2>outputs/148_Test.stderr.track
( echo "1 | featurevw 1" | ../build/vowpalwabbit/ ) >outputs/149_Test.stdout.track 2>outputs/149_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_adf -d train-sets/cb_test.ldf -p cb_adf_dm.predict --cb_type dm --sparse_weights ) >outputs/150_Test.stdout.track 2>outputs/150_Test.stderr.track
( ../build/vowpalwabbit/vw --lrqfa aa3 -d train-sets/0080.dat ) >outputs/151_Test.stdout.track 2>outputs/151_Test.stderr.track
( ./daemon-test.sh --foregroundvw  >vw-daemon.stdout 2>/dev/null ) >outputs/152_Test.stdout.track 2>outputs/152_Test.stderr.track
( ../build/vowpalwabbit/vw --marginal f  -d train-sets/marginal_features --noconstant --initial_numerator 0.5 --initial_denominator 1.0 --decay 0.001 --holdout_off -c -k --passes 100  --compete ) >outputs/153_Test.stdout.track 2>outputs/153_Test.stderr.track
( ../build/vowpalwabbit/vw -k --cache_file ignore_linear.cache --passes 10000 --holdout_off -d train-sets/0154.dat --noconstant --ignore_linear x -q xx ) >outputs/154_Test.stdout.track 2>outputs/154_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -i models/0097.model --save_resume --audit_regressor 0097.audit_regr ) >outputs/155_Test.stdout.track 2>outputs/155_Test.stderr.track
( ./cubic-test.sh $../build/vowpalwabbit/vwvw  >/dev/null 2>/dev/null ) >outputs/156_Test.stdout.track 2>outputs/156_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat -f models/sr.model  --passes 2 -c -k  -P 50 --save_resume ) >outputs/157_Test.stdout.track 2>outputs/157_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/decisionservice.json --dsjson --cb_explore_adf --epsilon 0.2 --quadratic GT -P 1 -p cbe_adf_dsjson.predict ) >outputs/158_Test.stdout.track 2>outputs/158_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_mini.dat --bootstrap 5 --binary -c -k --passes 2 ) >outputs/159_Test.stdout.track 2>outputs/159_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/multiclass --bootstrap 4 --oaa 10 -q  ) >outputs/160_Test.stdout.track 2>outputs/160_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/0001.dat --classweight 1 ) >outputs/161_Test.stdout.track 2>outputs/161_Test.stderr.track
( ../build/vowpalwabbit/vw --oaa 10 -d train-sets/multiclass --classweight 4 ) >outputs/162_Test.stdout.track 2>outputs/162_Test.stderr.track
( ../build/vowpalwabbit/vw --recall_tree 10 -d train-sets/multiclass --classweight 4 ) >outputs/163_Test.stdout.track 2>outputs/163_Test.stderr.track
( ../build/vowpalwabbit/vw --cs_active 3 -d ../test/train-sets/cs_test --cost_max 2 --mellowness 0.01 --simulation --adax ) >outputs/164_Test.stdout.track 2>outputs/164_Test.stderr.track
( ../build/vowpalwabbit/vw --cs_active 3 -d ../test/train-sets/cs_test --cost_max 2 --mellowness 1.0 --simulation --adax ) >outputs/165_Test.stdout.track 2>outputs/165_Test.stderr.track
( ../build/vowpalwabbit/vw --hash_seed 5 -d train-sets/rcv1_mini.dat --holdout_off --passes 2 -f hash_seed5.model -c -k --ngram 2 -q ::  ) >outputs/166_Test.stdout.track 2>outputs/166_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_mini.dat -i hash_seed5.model -t ) >outputs/167_Test.stdout.track 2>outputs/167_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_raw_cb_small. ) >outputs/168_Test.stdout.track 2>outputs/168_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_multiclass.dat --cbify 2 --epsilon 0.05 ) >outputs/169_Test.stdout.track 2>outputs/169_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 10 --cb_explore_adf --epsilon 0.05 -d train-sets/multiclass ) >outputs/170_Test.stdout.track 2>outputs/170_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 3 --cbify_cs --epsilon 0.05 -d train-sets/cs_cb ) >outputs/171_Test.stdout.track 2>outputs/171_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 3 --cbify_cs --cb_explore_adf --epsilon 0.05 -d train-sets/cs_cb ) >outputs/172_Test.stdout.track 2>outputs/172_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 10 --cb_explore_adf --cb_type mtr --regcb --mellowness 0.01 -d train-sets/multiclass ) >outputs/173_Test.stdout.track 2>outputs/173_Test.stderr.track
( ../build/vowpalwabbit/vw --cbify 10 --cb_explore_adf --cb_type mtr --regcbopt --mellowness 0.01 -d train-sets/multiclass ) >outputs/174_Test.stdout.track 2>outputs/174_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/cs_test.ldf --cbify_ldf --cb_type mtr --regcbopt --mellowness 0.01 ) >outputs/175_Test.stdout.track 2>outputs/175_Test.stderr.track
( ./same-model-test.shvw  >/dev/null 2>/dev/null ) >outputs/176_Test.stdout.track 2>outputs/176_Test.stderr.track
( printf '3 |f a b c |e x y z\n2 |f a y c |e x\n' | ../build/vowpalwabbit/vw --oaa 3 -q ef --audit ) >outputs/177_Test.stdout.track 2>outputs/177_Test.stderr.track
( ../build/vowpalwabbit/vw  --dsjson --cb_adf -d train-sets/no_shared_features.json ) >outputs/178_Test.stdout.track 2>outputs/178_Test.stderr.track
( ../build/vowpalwabbit/vw --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --warm_start_update --interaction_update -d train-sets/multiclass ) >outputs/179_Test.stdout.track 2>outputs/179_Test.stderr.track
( ../build/vowpalwabbit/vw --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --lambda_scheme 2 --warm_start_update --interaction_update -d train-sets/multiclass ) >outputs/180_Test.stdout.track 2>outputs/180_Test.stderr.track
( ../build/vowpalwabbit/vw --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --interaction_update -d train-sets/multiclass ) >outputs/181_Test.stdout.track 2>outputs/181_Test.stderr.track
( ../build/vowpalwabbit/vw --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.0 --warm_start 3 --interaction 7 --choices_lambda 8 --warm_start_update -d train-sets/multiclass ) >outputs/182_Test.stdout.track 2>outputs/182_Test.stderr.track
( ../build/vowpalwabbit/vw --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 1 --warm_start_update --interaction_update --sim_bandit -d train-sets/multiclass ) >outputs/183_Test.stdout.track 2>outputs/183_Test.stderr.track
( ../build/vowpalwabbit/vw --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --warm_start_update --interaction_update --corrupt_type_warm_start 2 --corrupt_prob_warm_start 0.5 -d train-sets/multiclass ) >outputs/184_Test.stdout.track 2>outputs/184_Test.stderr.track
( ../build/vowpalwabbit/vw --warm_cb 3 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 1 --interaction 2 --choices_lambda 8 --warm_start_update --interaction_update --warm_cb_cs -d train-sets/cs_cb ) >outputs/185_Test.stdout.track 2>outputs/185_Test.stderr.track
( ../build/vowpalwabbit/vw -k -P 100 --holdout_after 500 -d train-sets/0002.dat ) >outputs/186_Test.stdout.track 2>outputs/186_Test.stderr.track
( ../build/vowpalwabbit/vw -k -P 100 --holdout_after 500 -d train-sets/0002.dat -c --passes 2 ) >outputs/187_Test.stdout.track 2>outputs/187_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_adf --rank_all -d train-sets/cb_adf_sm.data -p cb_adf_sm.predict --cb_type sm ) >outputs/188_Test.stdout.track 2>outputs/188_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/b1848_dsjson_parser_regression.txt --dsjson --cb_explore_adf -P 1 ) >outputs/189_Test.stdout.track 2>outputs/189_Test.stderr.track
( ../build/vowpalwabbit/vw -k --oaa 10 --oaa_subsample 5 -c --passes 10 -d train-sets/multiclass --holdout_off ) >outputs/190_Test.stdout.track 2>outputs/190_Test.stderr.track
( ../build/vowpalwabbit/vw -k -d train-sets/0001.dat -f models/ftrl_coin.model --passes 1 --coin ) >outputs/191_Test.stdout.track 2>outputs/191_Test.stderr.track
( ../build/vowpalwabbit/vw -k -t -d train-sets/0001.dat -i models/ftrl_coin.model -p ftrl_coin.predict ) >outputs/192_Test.stdout.track 2>outputs/192_Test.stderr.track
( ./negative-test.sh ../build/vowpalwabbit/vw -d train-sets/malformed.dat --onethread --strict_parse ) >outputs/193_Test.stdout.track 2>outputs/193_Test.stderr.track
( ./negative-test.sh ../build/vowpalwabbit/vw -d train-sets/malformed.dat --strict_parse ) >outputs/194_Test.stdout.track 2>outputs/194_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/malformed.dat --onethread ) >outputs/195_Test.stdout.track 2>outputs/195_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_smaller.dat --memory_tree 10 --learn_at_leaf --max_number_of_labels 2 --dream_at_update 0 --dream_repeats 3 --online --leaf_example_multiplier 10 --alpha 0.1 -l 0.001 -b 15 --passes 1 --loss_function squared --holdout_off ) >outputs/196_Test.stdout.track 2>outputs/196_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/rcv1_smaller.dat --memory_tree 10 --learn_at_leaf --max_number_of_labels 2 --dream_at_update 0 --dream_repeats 3 --leaf_example_multiplier 10 --alpha 0.1 -l 0.001 -b 15 -c --passes 2 --loss_function squared --holdout_off ) >outputs/197_Test.stdout.track 2>outputs/197_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_sample --cb_explore_adf -d test-sets/cb_sample_seed.data -p cb_sample_seed.predict --random_seed 1234 ) >outputs/198_Test.stdout.track 2>outputs/198_Test.stderr.track
( ../build/vowpalwabbit/vw -d train-sets/ccb_test.dat --ccb_explore_adf -p ccb_test.predict ) >outputs/199_Test.stdout.track 2>outputs/199_Test.stderr.track
( ../build/vowpalwabbit/vw --cb_explore_adf --softmax --lambda 100000 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_softmax_biglambda.predict ) >outputs/200_Test.stdout.track 2>outputs/200_Test.stderr.track
( ../build/vowpalwabbit/vw --ccb_explore_adf --ring_size 7 -d train-sets/ccb_reuse_small.data ) >outputs/201_Test.stdout.track 2>outputs/201_Test.stderr.track
( ../build/vowpalwabbit/vw --ccb_explore_adf --ring_size 20 --dsjson -d train-sets/ccb_reuse_medium.dsjson ) >outputs/202_Test.stdout.track 2>outputs/202_Test.stderr.track
