

using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using System.IO.Compression;
using VW;

namespace cs_unittest
{
    [TestClass]
    public partial class RunTests : TestBase
    {
        [TestMethod]
        [Description(@"")]

		[TestCategory("Command Line")]
        public void CommandLine_Test1()
        {
            RunTestsHelper.ExecuteTest(
				1,
				"-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off",
				"train-sets/0001.dat",
				"train-sets/ref/0001.stderr",
				"");
        }

        [TestMethod]
        [Description(@"checking predictions as well")]

		[TestCategory("Command Line")]
        public void CommandLine_Test2()
        {
            RunTestsHelper.ExecuteTest(
				1,
				"-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off",
				"train-sets/0001.dat",
				"train-sets/ref/0001.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				2,
				"-k -t -d train-sets/0001.dat -i models/0001.model -p 0001.predict --invariant",
				"train-sets/0001.dat",
				"test-sets/ref/0001.stderr",
				"pred-sets/ref/0001.predict");
        }

        [TestMethod]
        [Description(@"without -d, training only")]

		[TestCategory("Command Line")]
        public void CommandLine_Test3()
        {
            RunTestsHelper.ExecuteTest(
				3,
				"-k -d train-sets/0002.dat -f models/0002.model --invariant",
				"train-sets/0002.dat",
				"train-sets/ref/0002.stderr",
				"");
        }

        [TestMethod]
        [Description(@"same, with -d")]

		[TestCategory("Command Line")]
        public void CommandLine_Test4()
        {
            RunTestsHelper.ExecuteTest(
				4,
				"-k -d train-sets/0002.dat -f models/0002.model --invariant",
				"train-sets/0002.dat",
				"train-sets/ref/0002.stderr",
				"");
        }

        [TestMethod]
        [Description(@"add -q .., adaptive, and more (same input, different outputs)")]

		[TestCategory("Command Line")]
        public void CommandLine_Test5()
        {
            RunTestsHelper.ExecuteTest(
				5,
				"-k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model -d train-sets/0002.dat",
				"train-sets/0002.dat",
				"train-sets/ref/0002a.stderr",
				"");
        }

        [TestMethod]
        [Description(@"run predictions on Test 4 model")]

		[TestCategory("Command Line")]
        public void CommandLine_Test6()
        {
            RunTestsHelper.ExecuteTest(
				4,
				"-k -d train-sets/0002.dat -f models/0002.model --invariant",
				"train-sets/0002.dat",
				"train-sets/ref/0002.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				6,
				"-k -t -i models/0002.model -d train-sets/0002.dat -p 0002b.predict",
				"train-sets/0002.dat",
				"test-sets/ref/0002b.stderr",
				"pred-sets/ref/0002b.predict");
        }

        [TestMethod]
        [Description(@"using normalized adaptive updates and a low --power_t")]

		[TestCategory("Command Line")]
        public void CommandLine_Test7()
        {
            RunTestsHelper.ExecuteTest(
				7,
				"-k --power_t 0.45 -f models/0002c.model -d train-sets/0002.dat",
				"train-sets/0002.dat",
				"train-sets/ref/0002c.stderr",
				"");
        }

        [TestMethod]
        [Description(@"predicts on test 7 model")]

		[TestCategory("Command Line")]
        public void CommandLine_Test8()
        {
            RunTestsHelper.ExecuteTest(
				7,
				"-k --power_t 0.45 -f models/0002c.model -d train-sets/0002.dat",
				"train-sets/0002.dat",
				"train-sets/ref/0002c.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				8,
				"-k -t -i models/0002c.model -d train-sets/0002.dat -p 0002c.predict",
				"train-sets/0002.dat",
				"test-sets/ref/0002c.stderr",
				"pred-sets/ref/0002c.predict");
        }

        [TestMethod]
        [Description(@"label-dependent features with csoaa_ldf")]

		[TestCategory("Command Line")]
        public void CommandLine_Test9()
        {
            RunTestsHelper.ExecuteTest(
				9,
				"-k -c -d train-sets/cs_test.ldf -p cs_test.ldf.csoaa.predict --passes 10 --invariant --csoaa_ldf multiline --holdout_off --noconstant",
				"train-sets/cs_test.ldf",
				"train-sets/ref/cs_test.ldf.csoaa.stderr",
				"train-sets/ref/cs_test.ldf.csoaa.predict");
        }

        [TestMethod]
        [Description(@"label-dependent features with wap_ldf")]

		[TestCategory("Command Line")]
        public void CommandLine_Test10()
        {
            RunTestsHelper.ExecuteTest(
				10,
				"-k -c -d train-sets/cs_test.ldf -p cs_test.ldf.wap.predict --passes 10 --invariant --wap_ldf multiline --holdout_off --noconstant",
				"train-sets/cs_test.ldf",
				"train-sets/ref/cs_test.ldf.wap.stderr",
				"train-sets/ref/cs_test.ldf.wap.predict");
        }

        [TestMethod]
        [Description(@"one-against-all")]

		[TestCategory("Command Line")]
        public void CommandLine_Test11()
        {
            RunTestsHelper.ExecuteTest(
				11,
				"-k --oaa 10 -c --passes 10 -d train-sets/multiclass --holdout_off",
				"train-sets/multiclass",
				"train-sets/ref/oaa.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Error Correcting Tournament")]

		[TestCategory("Command Line")]
        public void CommandLine_Test12()
        {
            RunTestsHelper.ExecuteTest(
				12,
				"-k --ect 10 --error 3 -c --passes 10 --invariant -d train-sets/multiclass --holdout_off",
				"train-sets/multiclass",
				"train-sets/ref/multiclass.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Run search (dagger) on wsj_small for 6 passes extra features")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test13()
        {
            RunTestsHelper.ExecuteTest(
				13,
				"-k -c -d train-sets/wsj_small.dat.gz --passes 6 --search_task sequence --search 45 --search_alpha 1e-6 --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1 --holdout_off",
				"train-sets/wsj_small.dat.gz",
				"train-sets/ref/search_wsj.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Run search (searn) on wsj_small for 6 passes extra features")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test14()
        {
            RunTestsHelper.ExecuteTest(
				14,
				"-k -c -d train-sets/wsj_small.dat.gz --passes 6 --search_task sequence --search 45 --search_alpha 1e-6 --search_max_bias_ngram_length 2 --search_max_quad_ngram_length 1 --holdout_off --search_passes_per_policy 3 --search_interpolation policy",
				"train-sets/wsj_small.dat.gz",
				"train-sets/ref/search_wsj2.dat.stderr",
				"");
        }

        [TestMethod]
        [Description(@"LBFGS on zero derivative input")]

		[TestCategory("Command Line")]
        public void CommandLine_Test15()
        {
            RunTestsHelper.ExecuteTest(
				15,
				"-k -c -d train-sets/zero.dat --loss_function=squared -b 20 --bfgs --mem 7 --passes 5 --l2 1.0 --holdout_off",
				"train-sets/zero.dat",
				"train-sets/ref/zero.stderr",
				"");
        }

        [TestMethod]
        [Description(@"LBFGS early termination")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test16()
        {
            RunTestsHelper.ExecuteTest(
				16,
				"-k -c -d train-sets/rcv1_small.dat --loss_function=logistic -b 20 --bfgs --mem 7 --passes 20 --termination 0.001 --l2 1.0 --holdout_off",
				"train-sets/rcv1_small.dat",
				"train-sets/ref/rcv1_small.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Run LDA with 100 topics on 1000 Wikipedia articles")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test17()
        {
            RunTestsHelper.ExecuteTest(
				17,
				"-k --lda 100 --lda_alpha 0.01 --lda_rho 0.01 --lda_D 1000 -l 1 -b 13 --minibatch 128 -d train-sets/wiki256.dat",
				"train-sets/wiki256.dat",
				"train-sets/ref/wiki1K.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Run search on seq_small for 12 passes, 4 passes per policy")]

		[TestCategory("Command Line")]
        public void CommandLine_Test18()
        {
            RunTestsHelper.ExecuteTest(
				18,
				"-k -c -d train-sets/seq_small --passes 12 --invariant --search 4 --search_task sequence --holdout_off",
				"train-sets/seq_small",
				"train-sets/ref/search_small.stderr",
				"");
        }

        [TestMethod]
        [Description(@"neural network 3-parity with 2 hidden units")]

		[TestCategory("Command Line")]
        public void CommandLine_Test19()
        {
            RunTestsHelper.ExecuteTest(
				19,
				"-k -c -d train-sets/3parity --hash all --passes 3000 -b 16 --nn 2 -l 10 --invariant -f models/0021.model --random_seed 19 --quiet --holdout_off",
				"train-sets/3parity",
				"train-sets/ref/3parity.stderr",
				"");
        }

        [TestMethod]
        [Description(@"neural network 3-parity with 2 hidden units (predict)")]

		[TestCategory("Command Line")]
        public void CommandLine_Test20()
        {
            RunTestsHelper.ExecuteTest(
				19,
				"-k -c -d train-sets/3parity --hash all --passes 3000 -b 16 --nn 2 -l 10 --invariant -f models/0021.model --random_seed 19 --quiet --holdout_off",
				"train-sets/3parity",
				"train-sets/ref/3parity.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				20,
				"-d train-sets/3parity --hash all -t -i models/0021.model -p 0022.predict",
				"train-sets/3parity",
				"pred-sets/ref/0022.stderr",
				"pred-sets/ref/0022.predict");
        }

        [TestMethod]
        [Description(@"cubic features -- on a parity test case")]

		[TestCategory("Command Line")]
        public void CommandLine_Test21()
        {
            RunTestsHelper.ExecuteTest(
				21,
				"-k -c -f models/xxor.model -d train-sets/xxor.dat --cubic abc --passes 100 --holdout_off --progress 1.33333",
				"train-sets/xxor.dat",
				"train-sets/ref/xxor.stderr",
				"");
        }

        [TestMethod]
        [Description(@"matrix factorization -- training")]

		[TestCategory("Command Line")]
        public void CommandLine_Test22()
        {
            RunTestsHelper.ExecuteTest(
				22,
				"-k -d train-sets/ml100k_small_train -b 16 -q ui --rank 10 --l2 2e-6 --learning_rate 0.05 --passes 2 --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg -c --loss_function classic --holdout_off",
				"train-sets/ml100k_small_train",
				"train-sets/ref/ml100k_small.stderr",
				"");
        }

        [TestMethod]
        [Description(@"matrix factorization -- testing")]

		[TestCategory("Command Line")]
        public void CommandLine_Test23()
        {
            RunTestsHelper.ExecuteTest(
				22,
				"-k -d train-sets/ml100k_small_train -b 16 -q ui --rank 10 --l2 2e-6 --learning_rate 0.05 --passes 2 --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg -c --loss_function classic --holdout_off",
				"train-sets/ml100k_small_train",
				"train-sets/ref/ml100k_small.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				23,
				"-i models/movielens.reg -t -d test-sets/ml100k_small_test",
				"test-sets/ml100k_small_test",
				"test-sets/ref/ml100k_small.stderr",
				"");
        }

        [TestMethod]
        [Description(@"active-learning -- training")]

		[TestCategory("Command Line")]
        public void CommandLine_Test24()
        {
            RunTestsHelper.ExecuteTest(
				24,
				"-k --active --simulation --mellowness 0.000001 -d train-sets/rcv1_small.dat -l 10 --initial_t 10 --random_seed 3",
				"train-sets/rcv1_small.dat",
				"train-sets/ref/active-simulation.t24.stderr",
				"");
        }

        [TestMethod]
        [Description(@"bagging -- training regressor")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test25()
        {
            RunTestsHelper.ExecuteTest(
				25,
				"-k -d train-sets/0002.dat -f models/bs.reg.model --bootstrap 4 -p bs.reg.predict",
				"train-sets/0002.dat",
				"train-sets/ref/bs.reg.stderr",
				"train-sets/ref/bs.reg.predict");
        }

        [TestMethod]
        [Description(@"bagging -- predicting with bagged regressor")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test26()
        {
            RunTestsHelper.ExecuteTest(
				25,
				"-k -d train-sets/0002.dat -f models/bs.reg.model --bootstrap 4 -p bs.reg.predict",
				"train-sets/0002.dat",
				"train-sets/ref/bs.reg.stderr",
				"train-sets/ref/bs.reg.predict");
            RunTestsHelper.ExecuteTest(
				26,
				"-d train-sets/0002.dat -i models/bs.reg.model -p bs.prreg.predict -t",
				"train-sets/0002.dat",
				"train-sets/ref/bs.prreg.stderr",
				"train-sets/ref/bs.prreg.predict");
        }

        [TestMethod]
        [Description(@"bagging -- binary classifiers")]

		[TestCategory("Command Line")]
        public void CommandLine_Test27()
        {
            RunTestsHelper.ExecuteTest(
				27,
				"-d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict",
				"train-sets/0001.dat",
				"train-sets/ref/bs.vote.stderr",
				"train-sets/ref/bs.vote.predict");
        }

        [TestMethod]
        [Description(@"bagging -- predict with bagged classifier")]

		[TestCategory("Command Line")]
        public void CommandLine_Test28()
        {
            RunTestsHelper.ExecuteTest(
				27,
				"-d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict",
				"train-sets/0001.dat",
				"train-sets/ref/bs.vote.stderr",
				"train-sets/ref/bs.vote.predict");
            RunTestsHelper.ExecuteTest(
				28,
				"-d train-sets/0001.dat -i models/bs.vote.model -p bs.prvote.predict -t",
				"train-sets/0001.dat",
				"train-sets/ref/bs.prvote.stderr",
				"train-sets/ref/bs.prvote.predict");
        }

        [TestMethod]
        [Description(@"affix features")]

		[TestCategory("Command Line")]
        public void CommandLine_Test29()
        {
            RunTestsHelper.ExecuteTest(
				29,
				"-d train-sets/affix_test.dat -k -c --passes 10 --holdout_off --affix -2",
				"train-sets/affix_test.dat",
				"train-sets/ref/affix_test.stderr",
				"");
        }

        [TestMethod]
        [Description(@"train --l1 regularized model")]

		[TestCategory("Command Line")]
        public void CommandLine_Test30()
        {
            RunTestsHelper.ExecuteTest(
				30,
				"-d train-sets/0001.dat -f models/mask.model --invert_hash mask.predict --l1 0.01",
				"train-sets/0001.dat",
				"train-sets/ref/mask.stderr",
				"");
        }

        [TestMethod]
        [Description(@"train model using --feature_mask")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test31()
        {
            RunTestsHelper.ExecuteTest(
				31,
				"-d train-sets/0001.dat --invert_hash remask.predict --feature_mask models/mask.model -f models/remask.model",
				"train-sets/0001.dat",
				"train-sets/ref/remask.stderr",
				"");
        }

        [TestMethod]
        [Description(@"train model using --feature_mask and --initial_regressor")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test32()
        {
            RunTestsHelper.ExecuteTest(
				31,
				"-d train-sets/0001.dat --invert_hash remask.predict --feature_mask models/mask.model -f models/remask.model",
				"train-sets/0001.dat",
				"train-sets/ref/remask.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				32,
				"-d train-sets/0001.dat --feature_mask models/mask.model -i models/remask.model",
				"train-sets/0001.dat",
				"train-sets/ref/remask.final.stderr",
				"");
        }

        [TestMethod]
        [Description(@"train model for topk recommender")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test33()
        {
            RunTestsHelper.ExecuteTest(
				33,
				"-d train-sets/topk.vw -f topk.model -q MF --passes 100 --cache_file topk-train.cache -k --holdout_off",
				"train-sets/topk.vw",
				"train-sets/ref/topk-train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"train model for topk recommender")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test34()
        {
            RunTestsHelper.ExecuteTest(
				33,
				"-d train-sets/topk.vw -f topk.model -q MF --passes 100 --cache_file topk-train.cache -k --holdout_off",
				"train-sets/topk.vw",
				"train-sets/ref/topk-train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				34,
				"-P 1 -d train-sets/topk.vw -i topk.model --top 2 -p topk-rec.predict",
				"train-sets/topk.vw",
				"train-sets/ref/topk-rec.stderr",
				"train-sets/ref/topk-rec.predict");
        }

        [TestMethod]
        [Description(@"non-centered data-set where constant >> 0")]

		[TestCategory("Command Line")]
        public void CommandLine_Test35()
        {
            RunTestsHelper.ExecuteTest(
				35,
				"-k --passes 100 -c --holdout_off --constant 1000 -d train-sets/big-constant.dat",
				"train-sets/big-constant.dat",
				"train-sets/ref/big-constant.stderr",
				"");
        }

        [TestMethod]
        [Description(@"new option: --progress w/ integer arg")]

		[TestCategory("Command Line")]
        public void CommandLine_Test36()
        {
            RunTestsHelper.ExecuteTest(
				36,
				"-k -d train-sets/0001.dat --progress 10",
				"train-sets/0001.dat",
				"train-sets/ref/progress-10.stderr",
				"");
        }

        [TestMethod]
        [Description(@"new-option: --progress w/ floating-point arg")]

		[TestCategory("Command Line")]
        public void CommandLine_Test37()
        {
            RunTestsHelper.ExecuteTest(
				37,
				"-k -d train-sets/0001.dat -P 0.5",
				"train-sets/0001.dat",
				"train-sets/ref/progress-0.5.stderr",
				"");
        }

        [TestMethod]
        [Description(@"--nn without --quiet to avoid nn regressions")]

		[TestCategory("Command Line")]
        public void CommandLine_Test38()
        {
            RunTestsHelper.ExecuteTest(
				38,
				"-k -d train-sets/0001.dat --nn 1",
				"train-sets/0001.dat",
				"train-sets/ref/nn-1-noquiet.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb with dr")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test39()
        {
            RunTestsHelper.ExecuteTest(
				39,
				"-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dr --ngram 2 --skips 4 -b 24 -l 0.25",
				"train-sets/rcv1_raw_cb_small.vw",
				"train-sets/ref/rcv1_raw_cb_dr.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb with ips")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test40()
        {
            RunTestsHelper.ExecuteTest(
				40,
				"-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type ips --ngram 2 --skips 4 -b 24 -l 0.125",
				"train-sets/rcv1_raw_cb_small.vw",
				"train-sets/ref/rcv1_raw_cb_ips.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb with dm")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test41()
        {
            RunTestsHelper.ExecuteTest(
				41,
				"-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dm --ngram 2 --skips 4 -b 24 -l 0.125",
				"train-sets/rcv1_raw_cb_small.vw",
				"train-sets/ref/rcv1_raw_cb_dm.stderr",
				"");
        }

        [TestMethod]
        [Description(@"--lda --passes 2 hang regression")]

		[TestCategory("Command Line")]
        public void CommandLine_Test42()
        {
            RunTestsHelper.ExecuteTest(
				42,
				"-k -d train-sets/lda-2pass-hang.dat --lda 10 -c --passes 2 --holdout_off",
				"train-sets/lda-2pass-hang.dat",
				"train-sets/ref/lda-2pass-hang.stderr",
				"");
        }

        [TestMethod]
        [Description(@"search sequence labeling, non-ldf train")]

		[TestCategory("Command Line")]
        public void CommandLine_Test43()
        {
            RunTestsHelper.ExecuteTest(
				43,
				"-k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequence --search 5 --holdout_off -f models/sequence_data.model",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.nonldf.train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"search sequence labeling, non-ldf test")]

		[TestCategory("Command Line")]
        public void CommandLine_Test44()
        {
            RunTestsHelper.ExecuteTest(
				43,
				"-k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequence --search 5 --holdout_off -f models/sequence_data.model",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.nonldf.train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				44,
				"-d train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.test.predict",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.nonldf.test.stderr",
				"train-sets/ref/sequence_data.nonldf.test.predict");
        }

        [TestMethod]
        [Description(@"make sure that history works")]

		[TestCategory("Command Line")]
        public void CommandLine_Test45()
        {
            RunTestsHelper.ExecuteTest(
				45,
				"-k -c -d train-sets/seq_small2 --passes 4 --search 4 --search_task sequence --holdout_off",
				"train-sets/seq_small2",
				"train-sets/ref/search_small2.stderr",
				"");
        }

        [TestMethod]
        [Description(@"search sequence labeling, ldf train")]

		[TestCategory("Command Line")]
        public void CommandLine_Test46()
        {
            RunTestsHelper.ExecuteTest(
				46,
				"-k -c -d train-sets/sequence_data --passes 20 --search_rollout ref --search_alpha 1e-8 --search_task sequence_demoldf --csoaa_ldf m --search 5 --holdout_off -f models/sequence_data.ldf.model --noconstant",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.ldf.train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"search sequence labeling, ldf test")]

		[TestCategory("Command Line")]
        public void CommandLine_Test47()
        {
            RunTestsHelper.ExecuteTest(
				46,
				"-k -c -d train-sets/sequence_data --passes 20 --search_rollout ref --search_alpha 1e-8 --search_task sequence_demoldf --csoaa_ldf m --search 5 --holdout_off -f models/sequence_data.ldf.model --noconstant",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.ldf.train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				47,
				"-d train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.test.predict --noconstant",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.ldf.test.stderr",
				"train-sets/ref/sequence_data.ldf.test.predict");
        }

        [TestMethod]
        [Description(@"search sequence SPAN labeling BIO, non-ldf train, no rollouts")]

		[TestCategory("Command Line")]
        public void CommandLine_Test48()
        {
            RunTestsHelper.ExecuteTest(
				48,
				"-k -c -d train-sets/sequencespan_data --passes 20 --invariant --search_rollout none --search_task sequencespan --search 7 --holdout_off -f models/sequencespan_data.model",
				"train-sets/sequencespan_data",
				"train-sets/ref/sequencespan_data.nonldf.train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"search sequence SPAN labeling BIO, non-ldf test")]

		[TestCategory("Command Line")]
        public void CommandLine_Test49()
        {
            RunTestsHelper.ExecuteTest(
				48,
				"-k -c -d train-sets/sequencespan_data --passes 20 --invariant --search_rollout none --search_task sequencespan --search 7 --holdout_off -f models/sequencespan_data.model",
				"train-sets/sequencespan_data",
				"train-sets/ref/sequencespan_data.nonldf.train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				49,
				"-d train-sets/sequencespan_data -t -i models/sequencespan_data.model -p sequencespan_data.nonldf.test.predict",
				"train-sets/sequencespan_data",
				"train-sets/ref/sequencespan_data.nonldf.test.stderr",
				"train-sets/ref/sequencespan_data.nonldf.test.predict");
        }

        [TestMethod]
        [Description(@"search sequence SPAN labeling BILOU, non-ldf train")]

		[TestCategory("Command Line")]
        public void CommandLine_Test50()
        {
            RunTestsHelper.ExecuteTest(
				50,
				"-k -c -d train-sets/sequencespan_data --passes 20 --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequencespan --search_span_bilou --search 7 --holdout_off -f models/sequencespan_data.model",
				"train-sets/sequencespan_data",
				"train-sets/ref/sequencespan_data.nonldf-bilou.train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"search sequence SPAN labeling BILOU, non-ldf test")]

		[TestCategory("Command Line")]
        public void CommandLine_Test51()
        {
            RunTestsHelper.ExecuteTest(
				50,
				"-k -c -d train-sets/sequencespan_data --passes 20 --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequencespan --search_span_bilou --search 7 --holdout_off -f models/sequencespan_data.model",
				"train-sets/sequencespan_data",
				"train-sets/ref/sequencespan_data.nonldf-bilou.train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				51,
				"-d train-sets/sequencespan_data -t --search_span_bilou -i models/sequencespan_data.model -p sequencespan_data.nonldf-bilou.test.predict",
				"train-sets/sequencespan_data",
				"train-sets/ref/sequencespan_data.nonldf-bilou.test.stderr",
				"train-sets/ref/sequencespan_data.nonldf-bilou.test.predict");
        }

        [TestMethod]
        [Description(@"silly test for ""argmax"" task")]

		[TestCategory("Command Line")]
        public void CommandLine_Test52()
        {
            RunTestsHelper.ExecuteTest(
				52,
				"-d train-sets/argmax_data -k -c --passes 20 --search_rollout ref --search_alpha 1e-8 --search_task argmax --search 2 --holdout_off",
				"train-sets/argmax_data",
				"train-sets/ref/argmax_data.stderr",
				"");
        }

        [TestMethod]
        [Description(@"(holdout-broken regression)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test53()
        {
            RunTestsHelper.ExecuteTest(
				53,
				"-k -c --passes 2 -d train-sets/0001.dat",
				"train-sets/0001.dat",
				"train-sets/ref/holdout-loss-not-zero.stderr",
				"");
        }

        [TestMethod]
        [Description(@"stagewise poly with exponent 0.25")]

		[TestCategory("Command Line")]
        public void CommandLine_Test54()
        {
            RunTestsHelper.ExecuteTest(
				54,
				"--stage_poly --sched_exponent 0.25 --batch_sz 1000 --batch_sz_no_doubling -d train-sets/rcv1_small.dat -p stage_poly.s025.predict --quiet",
				"train-sets/rcv1_small.dat",
				"train-sets/ref/stage_poly.s025.stderr",
				"train-sets/ref/stage_poly.s025.predict");
        }

        [TestMethod]
        [Description(@"stagewise poly with exponent 1.0")]

		[TestCategory("Command Line")]
        public void CommandLine_Test55()
        {
            RunTestsHelper.ExecuteTest(
				55,
				"--stage_poly --sched_exponent 1.0 --batch_sz 1000 --batch_sz_no_doubling -d train-sets/rcv1_small.dat --quiet",
				"train-sets/rcv1_small.dat",
				"train-sets/ref/stage_poly.s100.stderr",
				"");
        }

        [TestMethod]
        [Description(@"stagewise poly with exponent 0.25 and doubling batches")]

		[TestCategory("Command Line")]
        public void CommandLine_Test56()
        {
            RunTestsHelper.ExecuteTest(
				56,
				"--stage_poly --sched_exponent 0.25 --batch_sz 1000 -d train-sets/rcv1_small.dat -p stage_poly.s025.doubling.predict --quiet",
				"train-sets/rcv1_small.dat",
				"train-sets/ref/stage_poly.s025.doubling.stderr",
				"train-sets/ref/stage_poly.s025.doubling.predict");
        }

        [TestMethod]
        [Description(@"stagewise poly with exponent 1.0 and doubling batches")]

		[TestCategory("Command Line")]
        public void CommandLine_Test57()
        {
            RunTestsHelper.ExecuteTest(
				57,
				"--stage_poly --sched_exponent 1.0 --batch_sz 1000 -d train-sets/rcv1_small.dat -p stage_poly.s100.doubling.predict --quiet",
				"train-sets/rcv1_small.dat",
				"train-sets/ref/stage_poly.s100.doubling.stderr",
				"train-sets/ref/stage_poly.s100.doubling.predict");
        }

        [TestMethod]
        [Description(@"library test, train the initial model")]

		[TestCategory("Command Line")]
        public void CommandLine_Test58()
        {
            RunTestsHelper.ExecuteTest(
				58,
				"-c -k -d train-sets/library_train -f models/library_train.w -q st --passes 100 --hash all --noconstant --csoaa_ldf m --holdout_off",
				"train-sets/library_train",
				"train-sets/ref/library_train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"library test, run ezexample_predict")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test59()
        {
            RunTestsHelper.ExecuteTest(
				59,
				"",
				"",
				"train-sets/ref/ezexample_predict.stderr",
				"");
        }

        [TestMethod]
        [Description(@"empty test, bad builds (without make clean)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test60()
        {
            RunTestsHelper.ExecuteTest(
				60,
				"",
				"",
				"train-sets/ref/empty-set.stderr",
				"");
        }

        [TestMethod]
        [Description(@"daemon test")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test61()
        {
            RunTestsHelper.ExecuteTest(
				61,
				"",
				"",
				"",
				"");
        }

        [TestMethod]
        [Description(@"SVM linear kernel")]

		[TestCategory("Command Line")]
        public void CommandLine_Test62()
        {
            RunTestsHelper.ExecuteTest(
				62,
				"--ksvm --l2 1 --reprocess 5 -b 18 -p ksvm_train.linear.predict -d train-sets/rcv1_smaller.dat",
				"train-sets/rcv1_smaller.dat",
				"train-sets/ref/ksvm_train.linear.stderr",
				"train-sets/ref/ksvm_train.linear.predict");
        }

        [TestMethod]
        [Description(@"SVM polynomial kernel")]

		[TestCategory("Command Line")]
        public void CommandLine_Test63()
        {
            RunTestsHelper.ExecuteTest(
				63,
				"--ksvm --l2 1 --reprocess 5 -b 18 --kernel poly -p ksvm_train.poly.predict -d train-sets/rcv1_smaller.dat",
				"train-sets/rcv1_smaller.dat",
				"train-sets/ref/ksvm_train.poly.stderr",
				"train-sets/ref/ksvm_train.poly.predict");
        }

        [TestMethod]
        [Description(@"SVM rbf kernel")]

		[TestCategory("Command Line")]
        public void CommandLine_Test64()
        {
            RunTestsHelper.ExecuteTest(
				64,
				"--ksvm --l2 1 --reprocess 5 -b 18 --kernel rbf -p ksvm_train.rbf.predict -d train-sets/rcv1_smaller.dat",
				"train-sets/rcv1_smaller.dat",
				"train-sets/ref/ksvm_train.rbf.stderr",
				"train-sets/ref/ksvm_train.rbf.predict");
        }

        [TestMethod]
        [Description(@"Run search (dagger) on an entity-relation recognitions data set,")]

		[TestCategory("Command Line")]
        public void CommandLine_Test65()
        {
            RunTestsHelper.ExecuteTest(
				65,
				"-k -c -d train-sets/er_small.vw --passes 6 --search_task entity_relation --search 10 --constraints --search_alpha 1e-8",
				"train-sets/er_small.vw",
				"train-sets/ref/search_er.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Train a depenency parser with search (dagger)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test66()
        {
            RunTestsHelper.ExecuteTest(
				66,
				"-k -c -d train-sets/wsj_small.dparser.vw.gz --passes 6 --search_task dep_parser --search 12  --search_alpha 1e-4 --search_rollout oracle --holdout_off",
				"train-sets/wsj_small.dparser.vw.gz",
				"train-sets/ref/search_dep_parser.stderr",
				"");
        }

        [TestMethod]
        [Description(@"classification with data from dictionaries")]

		[TestCategory("Command Line")]
        public void CommandLine_Test67()
        {
            RunTestsHelper.ExecuteTest(
				67,
				"-k -c -d train-sets/dictionary_test.dat --binary --ignore w --holdout_off --passes 32 --dictionary w:dictionary_test.dict --dictionary w:dictionary_test.dict.gz --dictionary_path train-sets",
				"train-sets/dictionary_test.dat",
				"train-sets/ref/dictionary_test.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Search for multiclass classification")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test68()
        {
            RunTestsHelper.ExecuteTest(
				68,
				"-k -c -d train-sets/multiclass.sch --passes 20 --search_task multiclasstask --search 10 --search_alpha 1e-4 --holdout_off",
				"train-sets/multiclass.sch",
				"train-sets/ref/search_multiclass.stderr",
				"");
        }

        [TestMethod]
        [Description(@"(see Test 43/Test 44): search sequence labeling, with selective branching")]

		[TestCategory("Command Line")]
        public void CommandLine_Test69()
        {
            RunTestsHelper.ExecuteTest(
				43,
				"-k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout ref --search_alpha 1e-8 --search_task sequence --search 5 --holdout_off -f models/sequence_data.model",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.nonldf.train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				69,
				"-d train-sets/sequence_data -t -i models/sequence_data.model -p sequence_data.nonldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.nonldf.beam.test.stderr",
				"train-sets/ref/sequence_data.nonldf.beam.test.predict");
        }

        [TestMethod]
        [Description(@"(see Test 46/47) search sequence labeling, ldf test, with selective branching")]

		[TestCategory("Command Line")]
        public void CommandLine_Test70()
        {
            RunTestsHelper.ExecuteTest(
				46,
				"-k -c -d train-sets/sequence_data --passes 20 --search_rollout ref --search_alpha 1e-8 --search_task sequence_demoldf --csoaa_ldf m --search 5 --holdout_off -f models/sequence_data.ldf.model --noconstant",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.ldf.train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				70,
				"-d train-sets/sequence_data -t -i models/sequence_data.ldf.model -p sequence_data.ldf.beam.test.predict --search_metatask selective_branching --search_max_branch 10 --search_kbest 10 --noconstant",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.ldf.beam.test.stderr",
				"train-sets/ref/sequence_data.ldf.beam.test.predict");
        }

        [TestMethod]
        [Description(@"autolink")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test71()
        {
            RunTestsHelper.ExecuteTest(
				71,
				"-d train-sets/0002.dat --autolink 1 --examples 100 -p 0002.autolink.predict",
				"train-sets/0002.dat",
				"train-sets/ref/0002.autolink.stderr",
				"train-sets/ref/0002.autolink.predict");
        }

        [TestMethod]
        [Description(@"train FTRL-Proximal")]

		[TestCategory("Command Line")]
        public void CommandLine_Test72()
        {
            RunTestsHelper.ExecuteTest(
				72,
				"-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 1 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2",
				"train-sets/0001.dat",
				"train-sets/ref/0001_ftrl.stderr",
				"");
        }

        [TestMethod]
        [Description(@"test FTRL-Proximal")]

		[TestCategory("Command Line")]
        public void CommandLine_Test73()
        {
            RunTestsHelper.ExecuteTest(
				72,
				"-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 1 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2",
				"train-sets/0001.dat",
				"train-sets/ref/0001_ftrl.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				73,
				"-k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl.predict",
				"train-sets/0001.dat",
				"test-sets/ref/0001_ftrl.stderr",
				"pred-sets/ref/0001_ftrl.predict");
        }

        [TestMethod]
        [Description(@"cb evaluation")]

		[TestCategory("Command Line")]
        public void CommandLine_Test74()
        {
            RunTestsHelper.ExecuteTest(
				74,
				"-d train-sets/rcv1_cb_eval --cb 2 --eval",
				"train-sets/rcv1_cb_eval",
				"train-sets/ref/rcv1_cb_eval.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Log_multi")]

		[TestCategory("Command Line")]
        public void CommandLine_Test75()
        {
            RunTestsHelper.ExecuteTest(
				75,
				"--log_multi 10 -d train-sets/multiclass",
				"train-sets/multiclass",
				"train-sets/ref/log_multi.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cbify, epsilon-greedy")]

		[TestCategory("Command Line")]
        public void CommandLine_Test76()
        {
            RunTestsHelper.ExecuteTest(
				76,
				"--cbify 10 --epsilon 0.05 -d train-sets/multiclass",
				"train-sets/multiclass",
				"train-sets/ref/cbify_epsilon.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cbify, tau first")]

		[TestCategory("Command Line")]
        public void CommandLine_Test77()
        {
            RunTestsHelper.ExecuteTest(
				77,
				"--cbify 10 --first 5 -d train-sets/multiclass",
				"train-sets/multiclass",
				"train-sets/ref/cbify_first.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cbify, bag")]

		[TestCategory("Command Line")]
        public void CommandLine_Test78()
        {
            RunTestsHelper.ExecuteTest(
				78,
				"--cbify 10 --bag 7 -d train-sets/multiclass",
				"train-sets/multiclass",
				"train-sets/ref/cbify_bag.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cbify, cover")]

		[TestCategory("Command Line")]
        public void CommandLine_Test79()
        {
            RunTestsHelper.ExecuteTest(
				79,
				"--cbify 10 --cover 3 -d train-sets/multiclass",
				"train-sets/multiclass",
				"train-sets/ref/cbify_cover.stderr",
				"");
        }

        [TestMethod]
        [Description(@"lrq empty namespace")]

		[TestCategory("Command Line")]
        public void CommandLine_Test80()
        {
            RunTestsHelper.ExecuteTest(
				80,
				"--lrq aa3 -d train-sets/0080.dat",
				"train-sets/0080.dat",
				"train-sets/ref/0080.stderr",
				"");
        }

        [TestMethod]
        [Description(@"train FTRL-PiSTOL")]

		[TestCategory("Command Line")]
        public void CommandLine_Test81()
        {
            RunTestsHelper.ExecuteTest(
				81,
				"-k -d train-sets/0001.dat -f models/ftrl_pistol.model --passes 1 --pistol",
				"train-sets/0001.dat",
				"train-sets/ref/ftrl_pistol.stderr",
				"");
        }

        [TestMethod]
        [Description(@"test FTRL-PiSTOL")]

		[TestCategory("Command Line")]
        public void CommandLine_Test82()
        {
            RunTestsHelper.ExecuteTest(
				81,
				"-k -d train-sets/0001.dat -f models/ftrl_pistol.model --passes 1 --pistol",
				"train-sets/0001.dat",
				"train-sets/ref/ftrl_pistol.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				82,
				"-k -t -d train-sets/0001.dat -i models/ftrl_pistol.model -p ftrl_pistol.predict",
				"train-sets/0001.dat",
				"test-sets/ref/ftrl_pistol.stderr",
				"pred-sets/ref/ftrl_pistol.predict");
        }

        [TestMethod]
        [Description(@"check redefine functionality")]

		[TestCategory("Command Line")]
        public void CommandLine_Test83()
        {
            RunTestsHelper.ExecuteTest(
				83,
				"-k -d train-sets/0080.dat --redefine := --redefine y:=: --redefine x:=arma --ignore x -q yy",
				"train-sets/0080.dat",
				"train-sets/ref/redefine.stderr",
				"");
        }

        [TestMethod]
        [Description(@"check cb_adf")]

		[TestCategory("Command Line")]
        public void CommandLine_Test84()
        {
            RunTestsHelper.ExecuteTest(
				84,
				"--cb_adf -d train-sets/cb_test.ldf --noconstant",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cb_adf.stderr",
				"");
        }

        [TestMethod]
        [Description(@"check multilabel_oaa")]

		[TestCategory("Command Line")]
        public void CommandLine_Test85()
        {
            RunTestsHelper.ExecuteTest(
				85,
				"--multilabel_oaa 10 -d train-sets/multilabel -p multilabel.predict",
				"train-sets/multilabel",
				"train-sets/ref/multilabel.stderr",
				"pred-sets/ref/multilabel.predict");
        }

        [TestMethod]
        [Description(@"check --csoaa_rank on csoaa_ldf")]

		[TestCategory("Command Line")]
        public void CommandLine_Test86()
        {
            RunTestsHelper.ExecuteTest(
				86,
				"--csoaa_ldf multiline --csoaa_rank -d train-sets/cs_test_multilabel.ldf -p multilabel_ldf.predict --noconstant",
				"train-sets/cs_test_multilabel.ldf",
				"train-sets/ref/multilabel_ldf.stderr",
				"pred-sets/ref/multilabel_ldf.predict");
        }

        [TestMethod]
        [Description(@"check --rank_all on csoaa_ldf")]

		[TestCategory("Command Line")]
        public void CommandLine_Test87()
        {
            RunTestsHelper.ExecuteTest(
				87,
				"--cb_adf --rank_all -d train-sets/cb_test.ldf -p cb_adf_rank.predict --noconstant",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cb_adf_rank.stderr",
				"pred-sets/ref/cb_adf_rank.predict");
        }

        [TestMethod]
        [Description(@"named labels at training time")]

		[TestCategory("Command Line")]
        public void CommandLine_Test88()
        {
            RunTestsHelper.ExecuteTest(
				88,
				"--named_labels det,noun,verb --oaa 3 -d train-sets/test_named  -k -c --passes 10 --holdout_off -f models/test_named.model",
				"train-sets/test_named",
				"train-sets/ref/test_named_train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"named labels at prediction")]

		[TestCategory("Command Line")]
        public void CommandLine_Test89()
        {
            RunTestsHelper.ExecuteTest(
				88,
				"--named_labels det,noun,verb --oaa 3 -d train-sets/test_named  -k -c --passes 10 --holdout_off -f models/test_named.model",
				"train-sets/test_named",
				"train-sets/ref/test_named_train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				89,
				"-i models/test_named.model -t -d train-sets/test_named -p test_named.predict",
				"train-sets/test_named",
				"train-sets/ref/test_named_test.stderr",
				"pred-sets/ref/test_named.predict");
        }

        [TestMethod]
        [Description(@"named labels at training time (csoaa)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test90()
        {
            RunTestsHelper.ExecuteTest(
				90,
				"--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa  -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model",
				"train-sets/test_named_csoaa",
				"train-sets/ref/test_named_csoaa_train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"named labels at prediction (csoaa)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test91()
        {
            RunTestsHelper.ExecuteTest(
				90,
				"--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa  -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model",
				"train-sets/test_named_csoaa",
				"train-sets/ref/test_named_csoaa_train.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				91,
				"-i models/test_named_csoaa.model -t -d train-sets/test_named_csoaa -p test_named_csoaa.predict",
				"train-sets/test_named_csoaa",
				"train-sets/ref/test_named_csoaa_test.stderr",
				"pred-sets/ref/test_named_csoaa.predict");
        }

        [TestMethod]
        [Description(@"check -q :: and -oaa inverse hash")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test92()
        {
            RunTestsHelper.ExecuteTest(
				92,
				"",
				"",
				"train-sets/ref/inv_hash.stderr",
				"");
        }

        [TestMethod]
        [Description(@"check cb_adf with doubly robust option")]

		[TestCategory("Command Line")]
        public void CommandLine_Test93()
        {
            RunTestsHelper.ExecuteTest(
				93,
				"--cb_adf --rank_all -d train-sets/cb_test.ldf -p cb_adf_dr.predict --cb_type dr",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cb_adf_dr.stderr",
				"pred-sets/ref/cb_adf_dr.predict");
        }

        [TestMethod]
        [Description(@"experience replay version of test 1")]

		[TestCategory("Command Line")]
        public void CommandLine_Test94()
        {
            RunTestsHelper.ExecuteTest(
				94,
				"-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off --replay_b 100",
				"train-sets/0001.dat",
				"train-sets/ref/0001-replay.stderr",
				"");
        }

        [TestMethod]
        [Description(@"named labels at training time (csoaa) with experience replay")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test95()
        {
            RunTestsHelper.ExecuteTest(
				95,
				"--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model --replay_c 100",
				"train-sets/test_named_csoaa",
				"train-sets/ref/test_named_csoaa_train-replay.stderr",
				"");
        }

        [TestMethod]
        [Description(@"backwards compatibility")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test96()
        {
            RunTestsHelper.ExecuteTest(
				96,
				"",
				"",
				"test-sets/ref/backwards.stderr",
				"");
        }

        [TestMethod]
        [Description(@"")]

		[TestCategory("Command Line")]
        public void CommandLine_Test97()
        {
            RunTestsHelper.ExecuteTest(
				97,
				"-d train-sets/0001.dat -f models/0097.model --save_resume",
				"train-sets/0001.dat",
				"train-sets/ref/0097.stderr",
				"");
        }

        [TestMethod]
        [Description(@"checking predictions as well")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test98()
        {
            RunTestsHelper.ExecuteTest(
				97,
				"-d train-sets/0001.dat -f models/0097.model --save_resume",
				"train-sets/0001.dat",
				"train-sets/ref/0097.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				98,
				"-d train-sets/0001.dat -i models/0097.model -p 0098.predict",
				"train-sets/0001.dat",
				"test-sets/ref/0098.stderr",
				"pred-sets/ref/0098.predict");
        }

        [TestMethod]
        [Description(@"checking predictions with testing")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test99()
        {
            RunTestsHelper.ExecuteTest(
				97,
				"-d train-sets/0001.dat -f models/0097.model --save_resume",
				"train-sets/0001.dat",
				"train-sets/ref/0097.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				99,
				"-d train-sets/0001.dat -i models/0097.model -p 0099.predict",
				"train-sets/0001.dat",
				"test-sets/ref/0099.stderr",
				"pred-sets/ref/0099.predict");
        }

        [TestMethod]
        [Description(@"action costs, no rollout")]

		[TestCategory("Command Line")]
        public void CommandLine_Test100()
        {
            RunTestsHelper.ExecuteTest(
				100,
				"-k -c -d train-sets/sequence_data --passes 20 --invariant --search_rollout none --search_task sequence_ctg --search 5 --holdout_off",
				"train-sets/sequence_data",
				"train-sets/ref/sequence_data.ctg.train.stderr",
				"");
        }

        [TestMethod]
        [Description(@"active cover")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test101()
        {
            RunTestsHelper.ExecuteTest(
				101,
				"--loss_function logistic --binary --active_cover -d train-sets/rcv1_mini.dat -f models/active_cover.model",
				"train-sets/rcv1_mini.dat",
				"train-sets/ref/active_cover.stderr",
				"");
        }

        [TestMethod]
        [Description(@"active cover (predict)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test102()
        {
            RunTestsHelper.ExecuteTest(
				101,
				"--loss_function logistic --binary --active_cover -d train-sets/rcv1_mini.dat -f models/active_cover.model",
				"train-sets/rcv1_mini.dat",
				"train-sets/ref/active_cover.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				102,
				"-i models/active_cover.model --binary -t -d test-sets/rcv1_small_test.data -p active_cover.predict",
				"test-sets/rcv1_small_test.data",
				"test-sets/ref/active_cover.stderr",
				"pred-sets/ref/active_cover.predict");
        }

        [TestMethod]
        [Description(@"active cover oracular")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test103()
        {
            RunTestsHelper.ExecuteTest(
				103,
				"--loss_function logistic --binary --active_cover --oracular -d ./train-sets/rcv1_small.dat",
				"./train-sets/rcv1_small.dat",
				"train-sets/ref/active_cover_oracular.stderr",
				"");
        }

        [TestMethod]
        [Description(@"check cb_adf")]

		[TestCategory("Command Line")]
        public void CommandLine_Test104()
        {
            RunTestsHelper.ExecuteTest(
				104,
				"--cb_adf -d train-sets/cb_test.ldf --cb_type mtr --noconstant",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cb_adf_mtr.stderr",
				"");
        }

        [TestMethod]
        [Description(@"train FTRL-Proximal early stopping")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test105()
        {
            RunTestsHelper.ExecuteTest(
				105,
				"-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 3.0 --ftrl_beta 0 --l1 0.9 --cache",
				"train-sets/0001.dat",
				"train-sets/ref/0001_ftrl_holdout.stderr",
				"");
        }

        [TestMethod]
        [Description(@"test FTRL-Proximal early stopping prediction")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test106()
        {
            RunTestsHelper.ExecuteTest(
				105,
				"-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 3.0 --ftrl_beta 0 --l1 0.9 --cache",
				"train-sets/0001.dat",
				"train-sets/ref/0001_ftrl_holdout.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				106,
				"-k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout.predict",
				"train-sets/0001.dat",
				"test-sets/ref/0001_ftrl_holdout_106.stderr",
				"pred-sets/ref/0001_ftrl_holdout.predict");
        }

        [TestMethod]
        [Description(@"train FTRL-Proximal no early stopping")]

		[TestCategory("Command Line")]
        public void CommandLine_Test107()
        {
            RunTestsHelper.ExecuteTest(
				107,
				"-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off",
				"train-sets/0001.dat",
				"train-sets/ref/0001_ftrl_holdout_off.stderr",
				"");
        }

        [TestMethod]
        [Description(@"test FTRL-Proximal no early stopping")]

		[TestCategory("Command Line")]
        public void CommandLine_Test108()
        {
            RunTestsHelper.ExecuteTest(
				107,
				"-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off",
				"train-sets/0001.dat",
				"train-sets/ref/0001_ftrl_holdout_off.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				108,
				"-k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout_off.predict --holdout_off",
				"train-sets/0001.dat",
				"test-sets/ref/0001_ftrl_holdout_off.stderr",
				"pred-sets/ref/0001_ftrl_holdout_off.predict");
        }

        [TestMethod]
        [Description(@"--probabilities --oaa")]

		[TestCategory("Command Line")]
        public void CommandLine_Test109()
        {
            RunTestsHelper.ExecuteTest(
				109,
				"-d train-sets/probabilities.dat --probabilities --oaa=4 --loss_function=logistic -p oaa_probabilities.predict",
				"train-sets/probabilities.dat",
				"train-sets/ref/oaa_probabilities.stderr",
				"pred-sets/ref/oaa_probabilities.predict");
        }

        [TestMethod]
        [Description(@"--probabilities --csoaa_ldf=mc")]

		[TestCategory("Command Line")]
        public void CommandLine_Test110()
        {
            RunTestsHelper.ExecuteTest(
				110,
				"-d train-sets/cs_test.ldf --probabilities --csoaa_ldf=mc --loss_function=logistic -p csoaa_ldf_probabilities.predict",
				"train-sets/cs_test.ldf",
				"train-sets/ref/csoaa_ldf_probabilities.stderr",
				"pred-sets/ref/csoaa_ldf_probabilities.predict");
        }

        [TestMethod]
        [Description(@"Train a depenency parser with neural network and one_learner approach (lols)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test111()
        {
            RunTestsHelper.ExecuteTest(
				111,
				"-k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --search_task dep_parser --search 25 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout oracle --one_learner --nn 5 --ftrl --search_history_length 3 --root_label 8",
				"train-sets/wsj_small.dparser.vw.gz",
				"train-sets/ref/search_dep_parser_one_learner.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Train a depenency parser with cost_to_go")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test112()
        {
            RunTestsHelper.ExecuteTest(
				112,
				"-k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --passes 6 --search_task dep_parser --search 25 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout none --holdout_off --search_history_length 3 --root_label 8 --cost_to_go",
				"train-sets/wsj_small.dparser.vw.gz",
				"train-sets/ref/search_dep_parser_cost_to_go.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Predictions with confidences")]

		[TestCategory("Command Line")]
        public void CommandLine_Test113()
        {
            RunTestsHelper.ExecuteTest(
				113,
				"--confidence -d ./train-sets/rcv1_micro.dat --initial_t 0.1 -p confidence.preds",
				"./train-sets/rcv1_micro.dat",
				"train-sets/ref/confidence.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Over size example test")]

		[TestCategory("Command Line")]
        public void CommandLine_Test114()
        {
            RunTestsHelper.ExecuteTest(
				114,
				"-d train-sets/x.txt",
				"train-sets/x.txt",
				"train-sets/ref/oversize.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Long Line test")]

		[TestCategory("Command Line")]
        public void CommandLine_Test115()
        {
            RunTestsHelper.ExecuteTest(
				115,
				"-d train-sets/long_line -c -k",
				"train-sets/long_line",
				"train-sets/ref/long_line.stderr",
				"");
        }

        [TestMethod]
        [Description(@"MWT test")]

		[TestCategory("Command Line")]
        public void CommandLine_Test116()
        {
            RunTestsHelper.ExecuteTest(
				116,
				"-d train-sets/cb_eval --multiworld_test f -p cb_eval.preds",
				"train-sets/cb_eval",
				"train-sets/ref/cb_eval.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Audit regressor of ftrl model (from test #107)")]

		[TestCategory("Command Line")]
        public void CommandLine_Test117()
        {
            RunTestsHelper.ExecuteTest(
				107,
				"-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off",
				"train-sets/0001.dat",
				"train-sets/ref/0001_ftrl_holdout_off.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				117,
				"-d train-sets/0001.dat -i models/0001_ftrl.model  --audit_regressor ftrl.audit_regr",
				"train-sets/0001.dat",
				"train-sets/ref/ftrl_audit_regr.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Audit regressor of csoaa model (from test #95)")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test118()
        {
            RunTestsHelper.ExecuteTest(
				95,
				"--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model --replay_c 100",
				"train-sets/test_named_csoaa",
				"train-sets/ref/test_named_csoaa_train-replay.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				118,
				"-d train-sets/test_named_csoaa -i models/test_named_csoaa.model --audit_regressor csoaa.audit_regr",
				"train-sets/test_named_csoaa",
				"train-sets/ref/csoaa_audit_regr.stderr",
				"");
        }

        [TestMethod]
        [Description(@"MWT learn test")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test119()
        {
            RunTestsHelper.ExecuteTest(
				119,
				"-d train-sets/cb_eval --multiworld_test f --learn 2 -p mwt_learn.preds",
				"train-sets/cb_eval",
				"train-sets/ref/mwt_learn.stderr",
				"");
        }

        [TestMethod]
        [Description(@"MWT learn exclude test")]

		[Ignore]
		[TestCategory("Command Line")]
        public void CommandLine_Test120()
        {
            RunTestsHelper.ExecuteTest(
				120,
				"-d train-sets/cb_eval --multiworld_test f --learn 2 --exclude_eval -p mwt_learn_exclude.preds",
				"train-sets/cb_eval",
				"train-sets/ref/mwt_learn_exclude.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb_explore")]

		[TestCategory("Command Line")]
        public void CommandLine_Test121()
        {
            RunTestsHelper.ExecuteTest(
				121,
				"-d train-sets/rcv1_raw_cb_small.vw --cb_explore 2 --ngram 2 --skips 4 -b 24 -l 0.25 -p rcv1_raw_cb_explore.preds",
				"train-sets/rcv1_raw_cb_small.vw",
				"train-sets/ref/rcv1_raw_cb_explore.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Predictions with confidences after training")]

		[TestCategory("Command Line")]
        public void CommandLine_Test122()
        {
            RunTestsHelper.ExecuteTest(
				122,
				"--confidence --confidence_after_training --initial_t 0.1 -d ./train-sets/rcv1_small.dat -p confidence_after_training.preds",
				"./train-sets/rcv1_small.dat",
				"train-sets/ref/confidence_after_training.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb_eval save/load #1")]

		[TestCategory("Command Line")]
        public void CommandLine_Test123()
        {
            RunTestsHelper.ExecuteTest(
				123,
				"-d train-sets/cb_eval1 --multiworld_test f -f mwt.model -p cb_eval1.preds",
				"train-sets/cb_eval1",
				"train-sets/ref/cb_eval1.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb_eval save/load #2")]

		[TestCategory("Command Line")]
        public void CommandLine_Test124()
        {
            RunTestsHelper.ExecuteTest(
				123,
				"-d train-sets/cb_eval1 --multiworld_test f -f mwt.model -p cb_eval1.preds",
				"train-sets/cb_eval1",
				"train-sets/ref/cb_eval1.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				124,
				"-d train-sets/cb_eval2 -i mwt.model -p cb_eval2.preds",
				"train-sets/cb_eval2",
				"train-sets/ref/cb_eval2.stderr",
				"");
        }

        [TestMethod]
        [Description(@"arc-eager trasition-based dependency parser")]

		[TestCategory("Command Line")]
        public void CommandLine_Test125()
        {
            RunTestsHelper.ExecuteTest(
				125,
				"-k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --search_task dep_parser --search 26 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout oracle --one_learner --search_history_length 3 --root_label 8 --transition_system 2 --passes 8",
				"train-sets/wsj_small.dparser.vw.gz",
				"train-sets/ref/search_dep_parser_arceager.stderr",
				"");
        }

        [TestMethod]
        [Description(@"recall tree hello world")]

		[TestCategory("Command Line")]
        public void CommandLine_Test126()
        {
            RunTestsHelper.ExecuteTest(
				126,
				"--quiet -d train-sets/gauss1k.dat.gz -f models/recall_tree_g100.model --recall_tree 100 -b 20 --loss_function logistic ",
				"train-sets/gauss1k.dat.gz",
				"",
				"");
        }

        [TestMethod]
        [Description(@"recall_tree hello world predict-from-saved-model")]

		[TestCategory("Command Line")]
        public void CommandLine_Test127()
        {
            RunTestsHelper.ExecuteTest(
				126,
				"--quiet -d train-sets/gauss1k.dat.gz -f models/recall_tree_g100.model --recall_tree 100 -b 20 --loss_function logistic ",
				"train-sets/gauss1k.dat.gz",
				"",
				"");
            RunTestsHelper.ExecuteTest(
				127,
				"-t -d train-sets/gauss1k.dat.gz -i models/recall_tree_g100.model",
				"train-sets/gauss1k.dat.gz",
				"train-sets/ref/recall_tree_gauss1k.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb_explore_adf with epsilon-greedy exploration")]

		[TestCategory("Command Line")]
        public void CommandLine_Test128()
        {
            RunTestsHelper.ExecuteTest(
				128,
				"--cb_explore_adf --epsilon 0.1 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_epsilon.predict",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cbe_adf_epsilon.stderr",
				"pred-sets/ref/cbe_adf_epsilon.predict");
        }

        [TestMethod]
        [Description(@"cb_explore_adf with softmax exploration")]

		[TestCategory("Command Line")]
        public void CommandLine_Test129()
        {
            RunTestsHelper.ExecuteTest(
				129,
				"--cb_explore_adf --softmax --lambda 1 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_softmax.predict",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cbe_adf_softmax.stderr",
				"pred-sets/ref/cbe_adf_softmax.predict");
        }

        [TestMethod]
        [Description(@"cb_explore_adf with bagging exploration")]

		[TestCategory("Command Line")]
        public void CommandLine_Test130()
        {
            RunTestsHelper.ExecuteTest(
				130,
				"--cb_explore_adf --bag 3 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_bag.predict",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cbe_adf_bag.stderr",
				"pred-sets/ref/cbe_adf_bag.predict");
        }

        [TestMethod]
        [Description(@"cb_explore_adf with explore-first exploration")]

		[TestCategory("Command Line")]
        public void CommandLine_Test131()
        {
            RunTestsHelper.ExecuteTest(
				131,
				"--cb_explore_adf --first 2 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_first.predict",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cbe_adf_first.stderr",
				"pred-sets/ref/cbe_adf_first.predict");
        }

        [TestMethod]
        [Description(@"train a poisson model")]

		[TestCategory("Command Line")]
        public void CommandLine_Test132()
        {
            RunTestsHelper.ExecuteTest(
				132,
				"--quiet -d train-sets/poisson.dat -f models/poisson.model --loss_function poisson --link poisson -b 2 -p poisson.train.predict",
				"train-sets/poisson.dat",
				"train-sets/ref/poisson.train.stderr",
				"pred-sets/ref/poisson.train.predict");
        }

        [TestMethod]
        [Description(@"train a poisson model without invariant updates")]

		[TestCategory("Command Line")]
        public void CommandLine_Test133()
        {
            RunTestsHelper.ExecuteTest(
				133,
				"--quiet -d train-sets/poisson.dat -f models/poisson.normalized.model --normalized --loss_function poisson --link poisson -b 2 -l 0.1 -p poisson.train.normalized.predict",
				"train-sets/poisson.dat",
				"train-sets/ref/poisson.train.normalized.stderr",
				"pred-sets/ref/poisson.train.normalized.predict");
        }

        [TestMethod]
        [Description(@"second order online learning")]

		[TestCategory("Command Line")]
        public void CommandLine_Test134()
        {
            RunTestsHelper.ExecuteTest(
				134,
				"--OjaNewton -d train-sets/0001.dat -f models/second_order.model -p second_order.predict",
				"train-sets/0001.dat",
				"train-sets/ref/second_order.stderr",
				"pred-sets/ref/second_order.predict");
        }

        [TestMethod]
        [Description(@"cb explore adf")]

		[TestCategory("Command Line")]
        public void CommandLine_Test135()
        {
            RunTestsHelper.ExecuteTest(
				135,
				"-d train-sets/cb_adf_crash_1.data -f models/cb_adf_crash.model --cb_explore_adf --epsilon 0.05",
				"train-sets/cb_adf_crash_1.data",
				"train-sets/ref/cb_adf_crash1.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb explore adf predict")]

		[TestCategory("Command Line")]
        public void CommandLine_Test136()
        {
            RunTestsHelper.ExecuteTest(
				135,
				"-d train-sets/cb_adf_crash_1.data -f models/cb_adf_crash.model --cb_explore_adf --epsilon 0.05",
				"train-sets/cb_adf_crash_1.data",
				"train-sets/ref/cb_adf_crash1.stderr",
				"");
            RunTestsHelper.ExecuteTest(
				136,
				"-d train-sets/cb_adf_crash_2.data -i models/cb_adf_crash.model -t",
				"train-sets/cb_adf_crash_2.data",
				"train-sets/ref/cb_adf_crash2.stderr",
				"");
        }

        [TestMethod]
        [Description(@"Fix for regression introduced by badeedb.")]

		[TestCategory("Command Line")]
        public void CommandLine_Test137()
        {
            RunTestsHelper.ExecuteTest(
				137,
				"--audit -d train-sets/audit.dat --noconstant",
				"train-sets/audit.dat",
				"train-sets/ref/audit.stderr",
				"");
        }

        [TestMethod]
        [Description(@"cb_explore_adf with cover exploration")]

		[TestCategory("Command Line")]
        public void CommandLine_Test138()
        {
            RunTestsHelper.ExecuteTest(
				138,
				"--cb_explore_adf --cover 3 -d train-sets/cb_test.ldf --noconstant -p cbe_adf_cover.predict",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cbe_adf_cover.stderr",
				"pred-sets/ref/cbe_adf_cover.predict");
        }

        [TestMethod]
        [Description(@"cb_explore_adf with cover exploration + double robust")]

		[TestCategory("Command Line")]
        public void CommandLine_Test139()
        {
            RunTestsHelper.ExecuteTest(
				139,
				"--cb_explore_adf --cover 3 --cb_type dr -d train-sets/cb_test.ldf --noconstant -p cbe_adf_cover_dr.predict",
				"train-sets/cb_test.ldf",
				"train-sets/ref/cbe_adf_cover_dr.stderr",
				"pred-sets/ref/cbe_adf_cover_dr.predict");
        }

    }
}

