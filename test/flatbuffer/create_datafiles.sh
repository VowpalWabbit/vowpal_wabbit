mkdir -p train-sets/ref
mkdir -p test-sets/ref
mkdir -p pred-sets/ref
mkdir model-sets
mkdir models
cp -r ../train-sets/ref train-sets/
cp -r ../test-sets/ref test-sets/
cp -r ../pred-sets/ref pred-sets/
cp ../train-sets/dictionary_test.dict train-sets/dictionary_test.dict
cp ../train-sets/dictionary_test.dict.gz train-sets/dictionary_test.dict.gz
cp ../model-sets/* model-sets/
co ../models/* models/
echo "1" && ../../build/utl/flatbuffer/to_flatbuff -k -l 20 --initial_t 128000 --power_t 1 -d ../train-sets/0001.dat -f models/0001_1.model  --invariant   --holdout_off --flatout train-sets/0001.dat
echo "2" && ../../build/utl/flatbuffer/to_flatbuff -k -t -d ../train-sets/0001.dat -i models/0001_1.model -p 0001.predict --invariant --flatout train-sets/0001.dat
echo "3" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0002.dat -f models/0002.model --invariant --flatout train-sets/0002.dat
echo "4" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0002.dat -f models/0002.model --invariant --flatout train-sets/0002.dat
echo "5" && ../../build/utl/flatbuffer/to_flatbuff -k --initial_t 1 --adaptive --invariant   -f models/0002a.model -d ../train-sets/0002.dat --flatout train-sets/0002.dat
echo "6" && ../../build/utl/flatbuffer/to_flatbuff -k -t -i models/0002.model -d ../train-sets/0002.dat -p 0002b.predict --flatout train-sets/0002.dat
echo "7" && ../../build/utl/flatbuffer/to_flatbuff -k --power_t 0.45 -f models/0002c.model -d ../train-sets/0002.dat --flatout train-sets/0002.dat
echo "8" && ../../build/utl/flatbuffer/to_flatbuff -k -t -i models/0002c.model -d ../train-sets/0002.dat -p 0002c.predict --flatout train-sets/0002.dat
echo "9" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/cs_test.ldf -p cs_test.ldf.csoaa.predict  --invariant --csoaa_ldf multiline --holdout_off --noconstant --flatout train-sets/cs_test.ldf
echo "10" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/cs_test.ldf -p cs_test.ldf.wap.predict  --invariant --wap_ldf multiline --holdout_off --noconstant --flatout train-sets/cs_test.ldf
echo "11" && ../../build/utl/flatbuffer/to_flatbuff -k --oaa 10  -d ../train-sets/multiclass --holdout_off --flatout train-sets/multiclass
echo "12" && ../../build/utl/flatbuffer/to_flatbuff -k --ect 10 --error 3  --invariant -d ../train-sets/multiclass --holdout_off --flatout train-sets/multiclass
echo "13" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/wsj_small.dat.gz  --search_task sequence --search 45 --search_alpha 1e-6 --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1 --holdout_off --flatout train-sets/wsj_small.dat.gz
echo "14" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/wsj_small.dat.gz  --search_task sequence --search 45 --search_alpha 1e-6 --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1 --holdout_off --search_passes_per_policy 3 --search_interpolation policy --flatout train-sets/wsj_small.dat.gz
echo "15" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/zero.dat --loss_function=squared   --mem 7  --l2 1.0 --holdout_off --flatout train-sets/zero.dat
echo "16" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/rcv1_small.dat --loss_function=logistic  --mem 7  --termination 0.001 --l2 1.0 --holdout_off --flatout train-sets/rcv1_small.dat
echo "17" && ../../build/utl/flatbuffer/to_flatbuff -k --lda 100 --lda_alpha 0.01 --lda_rho 0.01 --lda_D 1000 -l 1  --minibatch 128 -d ../train-sets/wiki256.dat --flatout train-sets/wiki256.dat
echo "18" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/seq_small  --invariant --search 4 --search_task sequence --holdout_off --flatout train-sets/seq_small
echo "19" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/3parity --hash all   --nn 2 -l 10 --invariant -f models/0021.model --random_seed 19 --quiet --holdout_off --flatout train-sets/3parity
echo "20" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/3parity -t -i models/0021.model -p 0022.predict --flatout train-sets/3parity
echo "21" && ../../build/utl/flatbuffer/to_flatbuff -k -f models/xxor.model -d ../train-sets/xxor.dat --cubic abc  --holdout_off --progress 1.33333 --flatout train-sets/xxor.dat
echo "22" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/ml100k_small_train   --rank 10 --l2 2e-6 --learning_rate 0.05  --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg --loss_function classic --holdout_off --flatout train-sets/ml100k_small_train
echo "23" && ../../build/utl/flatbuffer/to_flatbuff -i models/movielens.reg -t -d ../test-sets/ml100k_small_test --flatout test-sets/ml100k_small_test
echo "24" && ../../build/utl/flatbuffer/to_flatbuff -k --active --simulation --mellowness 0.000001 -d ../train-sets/rcv1_small.dat -l 10 --initial_t 10 --random_seed 3 --flatout train-sets/rcv1_small.dat
echo "25" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0002.dat -f models/bs.reg.model --bootstrap 4 -p bs.reg.predict --flatout train-sets/0002.dat
echo "26" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0002.dat -i models/bs.reg.model -p bs.prreg.predict -t --flatout train-sets/0002.dat
echo "27" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict --flatout train-sets/0001.dat
echo "28" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat -i models/bs.vote.model -p bs.prvote.predict -t --flatout train-sets/0001.dat
echo "29" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/affix_test.dat -k  --holdout_off --affix -2 --flatout train-sets/affix_test.dat
echo "30" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat -f models/mask.model --invert_hash mask.predict --l1 0.01 --flatout train-sets/0001.dat
echo "31" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat --invert_hash remask.predict --feature_mask models/mask.model -f models/remask.model --flatout train-sets/0001.dat
echo "32" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat --feature_mask models/mask.model -i models/remask.model --flatout train-sets/0001.dat
echo "33" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/topk.vw -f topk.model   --cache_file topk-train.cache -k --holdout_off --flatout train-sets/topk.vw
echo "34" && ../../build/utl/flatbuffer/to_flatbuff -P 1 -d ../train-sets/topk.vw -i topk.model --top 2 -p topk-rec.predict --flatout train-sets/topk.vw
echo "35" && ../../build/utl/flatbuffer/to_flatbuff -k  --holdout_off --constant 1000 -d ../train-sets/big-constant.dat --flatout train-sets/big-constant.dat
echo "36" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat --progress 10 --flatout train-sets/0001.dat
echo "37" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat -P 0.5 --flatout train-sets/0001.dat
echo "38" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat --nn 1 --flatout train-sets/0001.dat
echo "39" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dr    -l 0.25 --flatout train-sets/rcv1_raw_cb_small.vw
echo "40" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type ips    -l 0.125 --flatout train-sets/rcv1_raw_cb_small.vw
echo "41" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dm    -l 0.125 -f cb_dm.reg --flatout train-sets/rcv1_raw_cb_small.vw
echo "42" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/lda-2pass-hang.dat --lda 10  --holdout_off --flatout train-sets/lda-2pass-hang.dat
echo "43" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/sequence_data  --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequence --search 5 --holdout_off -f models/sequence_data.model --flatout train-sets/sequence_data
echo "44" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.test.predict --flatout train-sets/sequence_data
echo "45" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/seq_small2  --search 4 --search_task sequence --holdout_off --flatout train-sets/seq_small2
echo "46" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/sequence_data  --search_rollout ref --search_alpha 1e-8 --search_task sequence_demoldf --csoaa_ldf m --search 5 --holdout_off -f models/sequence_data.ldf.model --noconstant --flatout train-sets/sequence_data
echo "47" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.test.predict --noconstant --flatout train-sets/sequence_data
echo "48" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/sequencespan_data  --invariant --search_rollout none --search_task sequencespan --search 7 --holdout_off -f models/sequencespan_data.model --flatout train-sets/sequencespan_data
echo "49" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/sequencespan_data -t -i models/sequencespan_data.model -p sequencespan_data.nonldf.test.predict --flatout train-sets/sequencespan_data
echo "50" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/sequencespan_data  --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequencespan --search_span_bilou --search 7 --holdout_off -f models/sequencespan_data.model --flatout train-sets/sequencespan_data
echo "51" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/sequencespan_data -t --search_span_bilou -i models/sequencespan_data.model -p sequencespan_data.nonldf-bilou.test.predict --flatout train-sets/sequencespan_data
echo "52" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/argmax_data -k  --search_rollout ref --search_alpha 1e-8 --search_task argmax --search 2 --holdout_off --flatout train-sets/argmax_data
echo "53" && ../../build/utl/flatbuffer/to_flatbuff -k  -d ../train-sets/0001.dat --flatout train-sets/0001.dat
echo "54" && ../../build/utl/flatbuffer/to_flatbuff --stage_poly --sched_exponent 0.25 --batch_sz 1000 --batch_sz_no_doubling -d ../train-sets/rcv1_small.dat -p stage_poly.s025.predict --quiet --flatout train-sets/rcv1_small.dat
echo "55" && ../../build/utl/flatbuffer/to_flatbuff --stage_poly --sched_exponent 1.0 --batch_sz 1000 --batch_sz_no_doubling -d ../train-sets/rcv1_small.dat --quiet --flatout train-sets/rcv1_small.dat
echo "56" && ../../build/utl/flatbuffer/to_flatbuff --stage_poly --sched_exponent 0.25 --batch_sz 1000 -d ../train-sets/rcv1_small.dat -p stage_poly.s025.doubling.predict --quiet --flatout train-sets/rcv1_small.dat
echo "57" && ../../build/utl/flatbuffer/to_flatbuff --stage_poly --sched_exponent 1.0 --batch_sz 1000 -d ../train-sets/rcv1_small.dat -p stage_poly.s100.doubling.predict --quiet --flatout train-sets/rcv1_small.dat
echo "58" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/library_train -f models/library_train.w   --hash all --noconstant --csoaa_ldf m --holdout_off --flatout train-sets/library_train
echo "59" && ../../build/utl/flatbuffer/to_flatbuff  --dsjson --cb_adf -d ../train-sets/no_shared_features.json --flatout train-sets/no_shared_features.json
echo "62" && ../../build/utl/flatbuffer/to_flatbuff --ksvm --l2 1 --reprocess 5  -p ksvm_train.linear.predict -d ../train-sets/rcv1_smaller.dat --flatout train-sets/rcv1_smaller.dat
echo "63" && ../../build/utl/flatbuffer/to_flatbuff --ksvm --l2 1 --reprocess 5  --kernel poly -p ksvm_train.poly.predict -d ../train-sets/rcv1_smaller.dat --flatout train-sets/rcv1_smaller.dat
echo "64" && ../../build/utl/flatbuffer/to_flatbuff --ksvm --l2 1 --reprocess 5  --kernel rbf -p ksvm_train.rbf.predict -d ../train-sets/rcv1_smaller.dat --flatout train-sets/rcv1_smaller.dat
echo "65" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/er_small.vw  --search_task entity_relation --search 10 --constraints --search_alpha 1e-8 --flatout train-sets/er_small.vw
echo "66" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/wsj_small.dparser.vw.gz  --search_task dep_parser --search 12  --search_alpha 1e-4 --search_rollout oracle --holdout_off --flatout train-sets/wsj_small.dparser.vw.gz
echo "67" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/dictionary_test.dat --binary --ignore w --holdout_off  --dictionary w:dictionary_test.dict --dictionary w:dictionary_test.dict.gz --dictionary_path train-sets --flatout train-sets/dictionary_test.dat
echo "68" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/multiclass.sch  --search_task multiclasstask --search 10 --search_alpha 1e-4 --holdout_off --flatout train-sets/multiclass.sch
echo "69" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10 --flatout train-sets/sequence_data
echo "70" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10 --noconstant --flatout train-sets/sequence_data
echo "71" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0002.dat --autolink 1 --examples 100 -p 0002.autolink.predict --flatout train-sets/0002.dat
echo "72" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat -f models/0001_ftrl.model  --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --flatout train-sets/0001.dat
echo "73" && ../../build/utl/flatbuffer/to_flatbuff -k -t -d ../train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl.predict --flatout train-sets/0001.dat
echo "74" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_cb_eval --cb 2 --eval --flatout train-sets/rcv1_cb_eval
echo "75" && ../../build/utl/flatbuffer/to_flatbuff --log_multi 10 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "76" && ../../build/utl/flatbuffer/to_flatbuff --cbify 10 --epsilon 0.05 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "77" && ../../build/utl/flatbuffer/to_flatbuff --cbify 10 --first 5 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "78" && ../../build/utl/flatbuffer/to_flatbuff --cbify 10 --bag 7 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "79" && ../../build/utl/flatbuffer/to_flatbuff --cbify 10 --cover 3 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "80" && ../../build/utl/flatbuffer/to_flatbuff --lrq aa3 -d ../train-sets/0080.dat --flatout train-sets/0080.dat
echo "81" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat -f models/ftrl_pistol.model  --pistol --flatout train-sets/0001.dat
echo "82" && ../../build/utl/flatbuffer/to_flatbuff -k -t -d ../train-sets/0001.dat -i models/ftrl_pistol.model -p ftrl_pistol.predict --flatout train-sets/0001.dat
echo "84" && ../../build/utl/flatbuffer/to_flatbuff --cb_adf -d ../train-sets/cb_test.ldf --noconstant --flatout train-sets/cb_test.ldf
echo "85" && ../../build/utl/flatbuffer/to_flatbuff --multilabel_oaa 10 -d ../train-sets/multilabel -p multilabel.predict --flatout train-sets/multilabel
echo "86" && ../../build/utl/flatbuffer/to_flatbuff --csoaa_ldf multiline --csoaa_rank -d ../train-sets/cs_test_multilabel.ldf -p multilabel_ldf.predict --noconstant --flatout train-sets/cs_test_multilabel.ldf
echo "87" && ../../build/utl/flatbuffer/to_flatbuff --cb_adf --rank_all -d ../train-sets/cb_test.ldf -p cb_adf_rank.predict --noconstant --flatout train-sets/cb_test.ldf
echo "90" && ../../build/utl/flatbuffer/to_flatbuff --named_labels det,noun,verb --csoaa 3 -d ../train-sets/test_named_csoaa  -k  --holdout_off -f models/test_named_csoaa.model --flatout train-sets/test_named_csoaa
echo "91" && ../../build/utl/flatbuffer/to_flatbuff -i models/test_named_csoaa.model -t -d ../train-sets/test_named_csoaa -p test_named_csoaa.predict --flatout train-sets/test_named_csoaa
echo "93" && ../../build/utl/flatbuffer/to_flatbuff --cb_adf --rank_all -d ../train-sets/cb_test.ldf -p cb_adf_dr.predict --cb_type dr --flatout train-sets/cb_test.ldf
echo "94" && ../../build/utl/flatbuffer/to_flatbuff -k -l 20 --initial_t 128000 --power_t 1 -d ../train-sets/0001.dat  --invariant   --holdout_off --replay_b 100 --flatout train-sets/0001.dat
echo "95" && ../../build/utl/flatbuffer/to_flatbuff --named_labels det,noun,verb --csoaa 3 -d ../train-sets/test_named_csoaa -k  --holdout_off -f models/test_named_csoaa.model --replay_c 100 --flatout train-sets/test_named_csoaa
echo "97" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat -f models/0097.model --save_resume --flatout train-sets/0001.dat
echo "98" && ../../build/utl/flatbuffer/to_flatbuff --preserve_performance_counters -d ../train-sets/0001.dat -i models/0097.model -p 0098.predict --flatout train-sets/0001.dat
echo "99" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat -i models/0097.model -p 0099.predict --flatout train-sets/0001.dat
echo "100" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/sequence_data  --invariant --search_rollout none --search_task sequence_ctg --search 5 --holdout_off --flatout train-sets/sequence_data
echo "101" && ../../build/utl/flatbuffer/to_flatbuff --loss_function logistic --binary --active_cover -d ../train-sets/rcv1_mini.dat -f models/active_cover.model --flatout train-sets/rcv1_mini.dat
echo "102" && ../../build/utl/flatbuffer/to_flatbuff -i models/active_cover.model -t -d ../test-sets/rcv1_small_test.data -p active_cover.predict --flatout test-sets/rcv1_small_test.data
echo "103" && ../../build/utl/flatbuffer/to_flatbuff --loss_function logistic --binary --active_cover --oracular -d .././train-sets/rcv1_small.dat --flatout ./train-sets/rcv1_small.dat
echo "104" && ../../build/utl/flatbuffer/to_flatbuff --cb_adf -d ../train-sets/cb_test.ldf --cb_type mtr --noconstant --flatout train-sets/cb_test.ldf
echo "105" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat -f models/0001_ftrl.model  --ftrl --ftrl_alpha 3.0 --ftrl_beta 0 --l1 0.9 --cache --flatout train-sets/0001.dat
echo "106" && ../../build/utl/flatbuffer/to_flatbuff -k -t -d ../train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout.predict --flatout train-sets/0001.dat
echo "107" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat -f models/0001_ftrl.model  --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off --flatout train-sets/0001.dat
echo "108" && ../../build/utl/flatbuffer/to_flatbuff -k -t -d ../train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout_off.predict --holdout_off --flatout train-sets/0001.dat
echo "109" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/probabilities.dat --probabilities --oaa=4 --loss_function=logistic -p oaa_probabilities.predict --flatout train-sets/probabilities.dat
echo "110" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cs_test.ldf --probabilities --csoaa_ldf=mc --loss_function=logistic -p csoaa_ldf_probabilities.predict --flatout train-sets/cs_test.ldf
echo "113" && ../../build/utl/flatbuffer/to_flatbuff --confidence -d .././train-sets/rcv1_micro.dat --initial_t 0.1 -p confidence.preds --flatout ./train-sets/rcv1_micro.dat
echo "114" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/x.txt --flatout train-sets/x.txt
echo "115" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/long_line -k --flatout train-sets/long_line
echo "116" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cb_eval --multiworld_test f -p cb_eval.preds --flatout train-sets/cb_eval
echo "117" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat -i models/0001_ftrl.model  _regressor ftrl.audit_regr --flatout train-sets/0001.dat
echo "118" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/test_named_csoaa -i models/test_named_csoaa.model _regressor csoaa.audit_regr --flatout train-sets/test_named_csoaa
echo "119" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cb_eval --multiworld_test f --learn 2 -p mwt_learn.preds --flatout train-sets/cb_eval
echo "120" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cb_eval --multiworld_test f --learn 2 --exclude_eval -p mwt_learn_exclude.preds --flatout train-sets/cb_eval
echo "121" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_raw_cb_small.vw --cb_explore 2    -l 0.25 -p rcv1_raw_cb_explore.preds --flatout train-sets/rcv1_raw_cb_small.vw
echo "122" && ../../build/utl/flatbuffer/to_flatbuff --confidence --confidence_after_training --initial_t 0.1 -d .././train-sets/rcv1_small.dat -p confidence_after_training.preds --flatout ./train-sets/rcv1_small.dat
echo "123" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cb_eval1 --multiworld_test f -f mwt.model -p cb_eval1.preds --flatout train-sets/cb_eval1
echo "124" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cb_eval2 -i mwt.model -p cb_eval2.preds --flatout train-sets/cb_eval2
echo "126" && ../../build/utl/flatbuffer/to_flatbuff --quiet -d ../train-sets/gauss1k.dat.gz -f models/recall_tree_g100.model --recall_tree 100  --loss_function logistic --flatout train-sets/gauss1k.dat.gz
echo "127" && ../../build/utl/flatbuffer/to_flatbuff -t -d ../train-sets/gauss1k.dat.gz -i models/recall_tree_g100.model --flatout train-sets/gauss1k.dat.gz
echo "128" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --epsilon 0.1 -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_epsilon.predict --flatout train-sets/cb_test.ldf
echo "129" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --softmax --lambda 1 -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_softmax.predict --flatout train-sets/cb_test.ldf
echo "130" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --bag 3 -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_bag.predict --flatout train-sets/cb_test.ldf
echo "131" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --first 2 -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_first.predict --flatout train-sets/cb_test.ldf
echo "132" && ../../build/utl/flatbuffer/to_flatbuff --quiet -d ../train-sets/poisson.dat -f models/poisson.model --loss_function poisson --link poisson  -p poisson.train.predict --flatout train-sets/poisson.dat
echo "133" && ../../build/utl/flatbuffer/to_flatbuff --quiet -d ../train-sets/poisson.dat -f models/poisson.normalized.model --normalized --loss_function poisson --link poisson  -l 0.1 -p poisson.train.normalized.predict --flatout train-sets/poisson.dat
echo "134" && ../../build/utl/flatbuffer/to_flatbuff --OjaNewton -d ../train-sets/0001.dat -f models/second_order.model -p second_order.predict --flatout train-sets/0001.dat
echo "135" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cb_adf_crash_1.data -f models/cb_adf_crash.model --cb_explore_adf --epsilon 0.05 --flatout train-sets/cb_adf_crash_1.data
echo "136" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cb_adf_crash_2.data -i models/cb_adf_crash.model -t --flatout train-sets/cb_adf_crash_2.data
echo "138" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --cover 3 -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_cover.predict --flatout train-sets/cb_test.ldf
echo "139" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --cover 3 --cb_type dr -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_cover_dr.predict --flatout train-sets/cb_test.ldf
echo "140" && ../../build/utl/flatbuffer/to_flatbuff --marginal f  -d ../train-sets/marginal_features --noconstant --initial_numerator 0.5 --initial_denominator 1.0 --decay 0.001 --holdout_off -k  -f marginal_model --flatout train-sets/marginal_features
echo "141" && ../../build/utl/flatbuffer/to_flatbuff -i marginal_model  -d ../train-sets/marginal_features --noconstant -t --flatout train-sets/marginal_features
echo "142" && ../../build/utl/flatbuffer/to_flatbuff --explore_eval --epsilon 0.2 -d ../train-sets/cb_test.ldf --noconstant -p explore_eval.predict --flatout train-sets/cb_test.ldf
echo "143" && ../../build/utl/flatbuffer/to_flatbuff -k -l 20 --initial_t 128000 --power_t 1 -d ../train-sets/0001.json --json  --invariant   --holdout_off --flatout train-sets/0001.json
echo "144" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --cover 3 --cb_type dr -d ../train-sets/cb_test.json --json --noconstant -p cbe_adf_cover_dr.predict --flatout train-sets/cb_test.json
echo "145" && ../../build/utl/flatbuffer/to_flatbuff --bootstrap 2 -d ../train-sets/labeled-unlabeled-mix.dat --flatout train-sets/labeled-unlabeled-mix.dat
echo "146" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --cover 3 --cb_type dr -d ../train-sets/cb_test256.json --json --noconstant -p cbe_adf_cover_dr256.predict --flatout train-sets/cb_test256.json
echo "147" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/probabilities.dat --scores --oaa=4 -p oaa_scores.predict --flatout train-sets/probabilities.dat
echo "148" && ../../build/utl/flatbuffer/to_flatbuff --cb_adf -d ../train-sets/cb_test.ldf -p cb_adf_dm.predict --cb_type dm --flatout train-sets/cb_test.ldf
echo "150" && ../../build/utl/flatbuffer/to_flatbuff --cb_adf -d ../train-sets/cb_test.ldf -p cb_adf_dm.predict --cb_type dm --sparse_weights --flatout train-sets/cb_test.ldf
echo "151" && ../../build/utl/flatbuffer/to_flatbuff --lrqfa aa3 -d ../train-sets/0080.dat --flatout train-sets/0080.dat
echo "153" && ../../build/utl/flatbuffer/to_flatbuff --marginal f  -d ../train-sets/marginal_features --noconstant --initial_numerator 0.5 --initial_denominator 1.0 --decay 0.001 --holdout_off -k   --compete --flatout train-sets/marginal_features
echo "154" && ../../build/utl/flatbuffer/to_flatbuff -k --cache_file ignore_linear.cache  --holdout_off -d ../train-sets/0154.dat --noconstant --ignore_linear x  --flatout train-sets/0154.dat
echo "157" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat -f models/sr.model   -k  -P 50 --save_resume --flatout train-sets/0001.dat
echo "158" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/decisionservice.json --dsjson --cb_explore_adf --epsilon 0.2 --quadratic GT -P 1 -p cbe_adf_dsjson.predict --flatout train-sets/decisionservice.json
echo "159" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_mini.dat --bootstrap 5 --binary -k  --flatout train-sets/rcv1_mini.dat
echo "160" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/multiclass --bootstrap 4 --oaa 10  --leave_duplicate_interactions  -k  --holdout_off -P1 --flatout train-sets/multiclass
echo "161" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/0001.dat --classweight 1:2,0:3.1,-1:5 --flatout train-sets/0001.dat
echo "162" && ../../build/utl/flatbuffer/to_flatbuff --oaa 10 -d ../train-sets/multiclass --classweight 4:0,7:0.1,2:10 --classweight 10:3 --flatout train-sets/multiclass
echo "163" && ../../build/utl/flatbuffer/to_flatbuff --recall_tree 10 -d ../train-sets/multiclass --classweight 4:0,7:0.1 --classweight 2:10,10:3 --flatout train-sets/multiclass
echo "164" && ../../build/utl/flatbuffer/to_flatbuff --cs_active 3 -d ../train-sets/cs_test --cost_max 2 --mellowness 0.01 --simulation --adax --flatout train-sets/cs_test
echo "165" && ../../build/utl/flatbuffer/to_flatbuff --cs_active 3 -d ../train-sets/cs_test --cost_max 2 --mellowness 1.0 --simulation --adax --flatout train-sets/cs_test
echo "166" && ../../build/utl/flatbuffer/to_flatbuff --hash_seed 5 -d ../train-sets/rcv1_mini.dat --holdout_off  -f hash_seed5.model -k   --flatout train-sets/rcv1_mini.dat
#echo "167" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_mini.dat -i hash_seed5.model -t --flatout train-sets/rcv1_mini.dat
#echo "168" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_raw_cb_small.vw -t -i cb_dm.reg --flatout train-sets/rcv1_raw_cb_small.vw
echo "169" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_multiclass.dat --cbify 2 --epsilon 0.05 --flatout train-sets/rcv1_multiclass.dat
echo "170" && ../../build/utl/flatbuffer/to_flatbuff --cbify 10 --cb_explore_adf --epsilon 0.05 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "171" && ../../build/utl/flatbuffer/to_flatbuff --cbify 3 --cbify_cs --epsilon 0.05 -d ../train-sets/cs_cb --flatout train-sets/cs_cb
echo "172" && ../../build/utl/flatbuffer/to_flatbuff --cbify 3 --cbify_cs --cb_explore_adf --epsilon 0.05 -d ../train-sets/cs_cb --flatout train-sets/cs_cb
echo "173" && ../../build/utl/flatbuffer/to_flatbuff --cbify 10 --cb_explore_adf --cb_type mtr --regcb --mellowness 0.01 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "174" && ../../build/utl/flatbuffer/to_flatbuff --cbify 10 --cb_explore_adf --cb_type mtr --regcbopt --mellowness 0.01 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "175" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/cs_test.ldf --cbify_ldf --cb_type mtr --regcbopt --mellowness 0.01 --flatout train-sets/cs_test.ldf
echo "178" && ../../build/utl/flatbuffer/to_flatbuff  --dsjson --cb_adf -d ../train-sets/no_shared_features.json --flatout train-sets/no_shared_features.json
echo "179" && ../../build/utl/flatbuffer/to_flatbuff --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --warm_start_update --interaction_update -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "180" && ../../build/utl/flatbuffer/to_flatbuff --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --lambda_scheme 2 --warm_start_update --interaction_update -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "181" && ../../build/utl/flatbuffer/to_flatbuff --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --interaction_update -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "182" && ../../build/utl/flatbuffer/to_flatbuff --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.0 --warm_start 3 --interaction 7 --choices_lambda 8 --warm_start_update -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "183" && ../../build/utl/flatbuffer/to_flatbuff --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 1 --warm_start_update --interaction_update --sim_bandit -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "184" && ../../build/utl/flatbuffer/to_flatbuff --warm_cb 10 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 3 --interaction 7 --choices_lambda 8 --warm_start_update --interaction_update --corrupt_type_warm_start 2 --corrupt_prob_warm_start 0.5 -d ../train-sets/multiclass --flatout train-sets/multiclass
echo "185" && ../../build/utl/flatbuffer/to_flatbuff --warm_cb 3 --cb_explore_adf --cb_type mtr --epsilon 0.05 --warm_start 1 --interaction 2 --choices_lambda 8 --warm_start_update --interaction_update --warm_cb_cs -d ../train-sets/cs_cb --flatout train-sets/cs_cb
echo "186" && ../../build/utl/flatbuffer/to_flatbuff -k -P 100 --holdout_after 500 -d ../train-sets/0002.dat --flatout train-sets/0002.dat
echo "187" && ../../build/utl/flatbuffer/to_flatbuff -k -P 100 --holdout_after 500 -d ../train-sets/0002.dat  --flatout train-sets/0002.dat
echo "188" && ../../build/utl/flatbuffer/to_flatbuff --cb_adf --rank_all -d ../train-sets/cb_adf_sm.data -p cb_adf_sm.predict --cb_type sm --flatout train-sets/cb_adf_sm.data
echo "189" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/b1848_dsjson_parser_regression.txt --dsjson --cb_explore_adf -P 1 --flatout train-sets/b1848_dsjson_parser_regression.txt
echo "190" && ../../build/utl/flatbuffer/to_flatbuff -k --oaa 10 --oaa_subsample 5  -d ../train-sets/multiclass --holdout_off --flatout train-sets/multiclass
echo "191" && ../../build/utl/flatbuffer/to_flatbuff -k -d ../train-sets/0001.dat -f models/ftrl_coin.model  --coin --flatout train-sets/0001.dat
echo "192" && ../../build/utl/flatbuffer/to_flatbuff -k -t -d ../train-sets/0001.dat -i models/ftrl_coin.model -p ftrl_coin.predict --flatout train-sets/0001.dat
echo "196" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_smaller.dat --memory_tree 10 --learn_at_leaf --max_number_of_labels 2 --dream_at_update 0 --dream_repeats 3 --online --leaf_example_multiplier 10 --alpha 0.1 -l 0.001   --loss_function squared --holdout_off --flatout train-sets/rcv1_smaller_memory_tree.dat
echo "197" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/rcv1_smaller.dat --memory_tree 10 --learn_at_leaf --max_number_of_labels 2 --dream_at_update 0 --dream_repeats 3 --leaf_example_multiplier 10 --alpha 0.1 -l 0.001   --loss_function squared --holdout_off --flatout train-sets/rcv1_smaller_memory_tree.dat
echo "199" && ../../build/utl/flatbuffer/to_flatbuff -d ../train-sets/ccb_test.dat --ccb_explore_adf -p ccb_test.predict --flatout train-sets/ccb_test.dat
echo "200" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --softmax --lambda 100000 -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_softmax_biglambda.predict --flatout train-sets/cb_test.ldf
echo "201" && ../../build/utl/flatbuffer/to_flatbuff --ccb_explore_adf --ring_size 7 -d ../train-sets/ccb_reuse_small.data --flatout train-sets/ccb_reuse_small.data
echo "204" && ../../build/utl/flatbuffer/to_flatbuff --classweight -1:0.5 --no_stdin
echo "205" && ../../build/utl/flatbuffer/to_flatbuff --cb_dro --cb_adf --rank_all -d ../train-sets/cb_adf_sm.data -p cb_dro_adf_sm.predict --cb_type sm --flatout train-sets/cb_adf_sm.data
echo "211" && ../../build/utl/flatbuffer/to_flatbuff --cb_explore_adf --rnd 1 -d ../train-sets/cb_test.ldf --noconstant -p cbe_adf_rnd.predict --flatout train-sets/cb_test.ldf