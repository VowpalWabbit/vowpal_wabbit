

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
            using (var vw = new VowpalWabbit("-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/0001.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"checking predictions as well")]
        [TestCategory("Command Line")]
        public void CommandLine_Test2()
        {
            using (var vw = new VowpalWabbit("-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-k -t -d train-sets/0001.dat -i models/0001.model -p 0001.predict --invariant"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("test-sets/ref/0001.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"without -d, training only")]
        [TestCategory("Command Line")]
        public void CommandLine_Test3()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0002.dat -f models/0002.model --invariant"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/0002.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"same, with -d")]
        [TestCategory("Command Line")]
        public void CommandLine_Test4()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0002.dat -f models/0002.model --invariant"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/0002.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"add -q .., adaptive, and more (same input, different outputs)")]
        [TestCategory("Command Line")]
        public void CommandLine_Test5()
        {
            using (var vw = new VowpalWabbit("-k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model -d train-sets/0002.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/0002a.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"run predictions on Test 4 model")]
        [TestCategory("Command Line")]
        public void CommandLine_Test6()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0002.dat -f models/0002.model --invariant"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                							}
            using (var vw = new VowpalWabbit("-k -t -i models/0002.model -d train-sets/0002.dat -p 0002b.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("test-sets/ref/0002b.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"using normalized adaptive updates and a low --power_t")]
        [TestCategory("Command Line")]
        public void CommandLine_Test7()
        {
            using (var vw = new VowpalWabbit("-k --power_t 0.45 -f models/0002c.model -d train-sets/0002.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/0002c.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"predicts on test 7 model")]
        [TestCategory("Command Line")]
        public void CommandLine_Test8()
        {
            using (var vw = new VowpalWabbit("-k --power_t 0.45 -f models/0002c.model -d train-sets/0002.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                							}
            using (var vw = new VowpalWabbit("-k -t -i models/0002c.model -d train-sets/0002.dat -p 0002c.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0002.dat"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("test-sets/ref/0002c.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"one-against-all")]
        [TestCategory("Command Line")]
        public void CommandLine_Test11()
        {
            using (var vw = new VowpalWabbit("-k --oaa 10 -c --passes 10 -d train-sets/multiclass --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/multiclass"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/oaa.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"Error Correcting Tournament")]
        [TestCategory("Command Line")]
        public void CommandLine_Test12()
        {
            using (var vw = new VowpalWabbit("-k --ect 10 --error 3 -c --passes 10 --invariant -d train-sets/multiclass --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/multiclass"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/multiclass.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"LBFGS on zero derivative input")]
        [TestCategory("Command Line")]
        public void CommandLine_Test15()
        {
            using (var vw = new VowpalWabbit("-k -c -d train-sets/zero.dat --loss_function=squared -b 20 --bfgs --mem 7 --passes 5 --l2 1.0 --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/zero.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/zero.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cubic features -- on a parity test case")]
        [TestCategory("Command Line")]
        public void CommandLine_Test21()
        {
            using (var vw = new VowpalWabbit("-k -c -f models/xxor.model -d train-sets/xxor.dat --cubic abc --passes 100 --holdout_off --progress 1.33333"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/xxor.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/xxor.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"matrix factorization -- training")]
        [TestCategory("Command Line")]
        public void CommandLine_Test22()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/ml100k_small_train -b 16 -q ui --rank 10 --l2 2e-6 --learning_rate 0.05 --passes 2 --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg -c --loss_function classic --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/ml100k_small_train"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/ml100k_small.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"matrix factorization -- testing")]
        [TestCategory("Command Line")]
        public void CommandLine_Test23()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/ml100k_small_train -b 16 -q ui --rank 10 --l2 2e-6 --learning_rate 0.05 --passes 2 --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg -c --loss_function classic --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/ml100k_small_train"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-i models/movielens.reg -t -d test-sets/ml100k_small_test"))
            {
				                foreach (var dataLine in File.ReadLines(@"test-sets/ml100k_small_test"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("test-sets/ref/ml100k_small.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"bagging -- binary classifiers")]
        [TestCategory("Command Line")]
        public void CommandLine_Test27()
        {
            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/bs.vote.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"bagging -- predict with bagged classifier")]
        [TestCategory("Command Line")]
        public void CommandLine_Test28()
        {
            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                							}
            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -i models/bs.vote.model -p bs.prvote.predict -t"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/bs.prvote.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"affix features")]
        [TestCategory("Command Line")]
        public void CommandLine_Test29()
        {
            using (var vw = new VowpalWabbit("-d train-sets/affix_test.dat -k -c --passes 10 --holdout_off --affix -2"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/affix_test.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/affix_test.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"train --l1 regularized model")]
        [TestCategory("Command Line")]
        public void CommandLine_Test30()
        {
            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -f models/mask.model --invert_hash mask.predict --l1 0.01"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/mask.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"non-centered data-set where constant >> 0")]
        [TestCategory("Command Line")]
        public void CommandLine_Test35()
        {
            using (var vw = new VowpalWabbit("-k --passes 100 -c --holdout_off --constant 1000 -d train-sets/big-constant.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/big-constant.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/big-constant.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"new option: --progress w/ integer arg")]
        [TestCategory("Command Line")]
        public void CommandLine_Test36()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat --progress 10"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/progress-10.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"new-option: --progress w/ floating-point arg")]
        [TestCategory("Command Line")]
        public void CommandLine_Test37()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -P 0.5"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/progress-0.5.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"--nn without --quiet to avoid nn regressions")]
        [TestCategory("Command Line")]
        public void CommandLine_Test38()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat --nn 1"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/nn-1-noquiet.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cb with dr")]
        [TestCategory("Command Line")]
        public void CommandLine_Test39()
        {
            using (var vw = new VowpalWabbit("-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dr --ngram 2 --skips 4 -b 24 -l 0.25"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/rcv1_raw_cb_small.vw"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/rcv1_raw_cb_dr.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cb with ips")]
        [TestCategory("Command Line")]
        public void CommandLine_Test40()
        {
            using (var vw = new VowpalWabbit("-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type ips --ngram 2 --skips 4 -b 24 -l 0.125"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/rcv1_raw_cb_small.vw"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/rcv1_raw_cb_ips.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cb with dm")]
        [TestCategory("Command Line")]
        public void CommandLine_Test41()
        {
            using (var vw = new VowpalWabbit("-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dm --ngram 2 --skips 4 -b 24 -l 0.125"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/rcv1_raw_cb_small.vw"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/rcv1_raw_cb_dm.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"SVM linear kernel")]
        [TestCategory("Command Line")]
        public void CommandLine_Test62()
        {
            using (var vw = new VowpalWabbit("--ksvm --l2 1 --reprocess 5 -b 18 -p ksvm_train.linear.predict -d train-sets/rcv1_smaller.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/rcv1_smaller.dat"))
				                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/ksvm_train.linear.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"SVM polynomial kernel")]
        [TestCategory("Command Line")]
        public void CommandLine_Test63()
        {
            using (var vw = new VowpalWabbit("--ksvm --l2 1 --reprocess 5 -b 18 --kernel poly -p ksvm_train.poly.predict -d train-sets/rcv1_smaller.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/rcv1_smaller.dat"))
				                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/ksvm_train.poly.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"SVM rbf kernel")]
        [TestCategory("Command Line")]
        public void CommandLine_Test64()
        {
            using (var vw = new VowpalWabbit("--ksvm --l2 1 --reprocess 5 -b 18 --kernel rbf -p ksvm_train.rbf.predict -d train-sets/rcv1_smaller.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/rcv1_smaller.dat"))
				                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/ksvm_train.rbf.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"classification with data from dictionaries")]
        [TestCategory("Command Line")]
        public void CommandLine_Test67()
        {
            using (var vw = new VowpalWabbit("-k -c -d train-sets/dictionary_test.dat --binary --ignore w --holdout_off --passes 32 --dictionary w:dictionary_test.dict --dictionary w:dictionary_test.dict.gz --dictionary_path train-sets"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/dictionary_test.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/dictionary_test.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"train FTRL-Proximal")]
        [TestCategory("Command Line")]
        public void CommandLine_Test72()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 1 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/0001_ftrl.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"test FTRL-Proximal")]
        [TestCategory("Command Line")]
        public void CommandLine_Test73()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 1 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("test-sets/ref/0001_ftrl.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cb evaluation")]
        [TestCategory("Command Line")]
        public void CommandLine_Test74()
        {
            using (var vw = new VowpalWabbit("-d train-sets/rcv1_cb_eval --cb 2 --eval"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/rcv1_cb_eval"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/rcv1_cb_eval.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"Log_multi")]
        [TestCategory("Command Line")]
        public void CommandLine_Test75()
        {
            using (var vw = new VowpalWabbit("--log_multi 10 -d train-sets/multiclass"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/multiclass"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/log_multi.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cbify, epsilon-greedy")]
        [TestCategory("Command Line")]
        public void CommandLine_Test76()
        {
            using (var vw = new VowpalWabbit("--cbify 10 --epsilon 0.05 -d train-sets/multiclass"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/multiclass"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/cbify_epsilon.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cbify, bag")]
        [TestCategory("Command Line")]
        public void CommandLine_Test78()
        {
            using (var vw = new VowpalWabbit("--cbify 10 --bag 7 -d train-sets/multiclass"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/multiclass"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/cbify_bag.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"cbify, cover")]
        [TestCategory("Command Line")]
        public void CommandLine_Test79()
        {
            using (var vw = new VowpalWabbit("--cbify 10 --cover 3 -d train-sets/multiclass"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/multiclass"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/cbify_cover.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"lrq empty namespace")]
        [TestCategory("Command Line")]
        public void CommandLine_Test80()
        {
            using (var vw = new VowpalWabbit("--lrq aa3 -d train-sets/0080.dat"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0080.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/0080.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"train FTRL-PiSTOL")]
        [TestCategory("Command Line")]
        public void CommandLine_Test81()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/ftrl_pistol.model --passes 1 --pistol"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/ftrl_pistol.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"test FTRL-PiSTOL")]
        [TestCategory("Command Line")]
        public void CommandLine_Test82()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/ftrl_pistol.model --passes 1 --pistol"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-k -t -d train-sets/0001.dat -i models/ftrl_pistol.model -p ftrl_pistol.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("test-sets/ref/ftrl_pistol.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"check redefine functionality")]
        [TestCategory("Command Line")]
        public void CommandLine_Test83()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0080.dat --redefine := --redefine y:=: --redefine x:=arma --ignore x -q yy"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0080.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/redefine.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"named labels at training time")]
        [TestCategory("Command Line")]
        public void CommandLine_Test88()
        {
            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --oaa 3 -d train-sets/test_named  -k -c --passes 10 --holdout_off -f models/test_named.model"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/test_named_train.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"named labels at prediction")]
        [TestCategory("Command Line")]
        public void CommandLine_Test89()
        {
            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --oaa 3 -d train-sets/test_named  -k -c --passes 10 --holdout_off -f models/test_named.model"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-i models/test_named.model -t -d train-sets/test_named -p test_named.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/test_named_test.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"named labels at training time (csoaa)")]
        [TestCategory("Command Line")]
        public void CommandLine_Test90()
        {
            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa  -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named_csoaa"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/test_named_csoaa_train.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"named labels at prediction (csoaa)")]
        [TestCategory("Command Line")]
        public void CommandLine_Test91()
        {
            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa  -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named_csoaa"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-i models/test_named_csoaa.model -t -d train-sets/test_named_csoaa -p test_named_csoaa.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named_csoaa"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/test_named_csoaa_test.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"experience replay version of test 1")]
        [TestCategory("Command Line")]
        public void CommandLine_Test94()
        {
            using (var vw = new VowpalWabbit("-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off --replay_b 100"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/0001-replay.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"named labels at training time (csoaa) with experience replay")]
        [TestCategory("Command Line")]
        public void CommandLine_Test95()
        {
            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model --replay_c 100"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named_csoaa"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/test_named_csoaa_train-replay.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"")]
        [TestCategory("Command Line")]
        public void CommandLine_Test97()
        {
            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -f models/0097.model --save_resume"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/0097.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"train FTRL-Proximal no early stopping")]
        [TestCategory("Command Line")]
        public void CommandLine_Test107()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                                VWTestHelper.AssertEqual("train-sets/ref/0001_ftrl_holdout_off.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"test FTRL-Proximal no early stopping")]
        [TestCategory("Command Line")]
        public void CommandLine_Test108()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-k -t -d train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl_holdout_off.predict --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("test-sets/ref/0001_ftrl_holdout_off.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"--probabilities --oaa")]
        [TestCategory("Command Line")]
        public void CommandLine_Test109()
        {
            using (var vw = new VowpalWabbit("-d train-sets/probabilities.dat --probabilities --oaa=4 --loss_function=logistic -p oaa_probabilities.predict"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/probabilities.dat"))
				                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/oaa_probabilities.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"Predictions with confidences")]
        [TestCategory("Command Line")]
        public void CommandLine_Test113()
        {
            using (var vw = new VowpalWabbit("--confidence -d ./train-sets/rcv1_micro.dat --initial_t 0.1 -p confidence.preds"))
            {
				                foreach (var dataLine in File.ReadLines(@"./train-sets/rcv1_micro.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/confidence.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"Audit regressor of ftrl model (from test #107)")]
        [TestCategory("Command Line")]
        public void CommandLine_Test117()
        {
            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -i models/0001_ftrl.model  --audit_regressor ftrl.audit_regr"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/0001.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/ftrl_audit_regr.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"Audit regressor of csoaa model (from test #95)")]
        [TestCategory("Command Line")]
        public void CommandLine_Test118()
        {
            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model --replay_c 100"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named_csoaa"))
				                {
                    vw.Learn(dataLine);
                }
                                vw.RunMultiPass();
                                							}
            using (var vw = new VowpalWabbit("-d train-sets/test_named_csoaa -i models/test_named_csoaa.model --audit_regressor csoaa.audit_regr"))
            {
				                foreach (var dataLine in File.ReadLines(@"train-sets/test_named_csoaa"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/csoaa_audit_regr.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"Predictions with confidences after training")]
        [TestCategory("Command Line")]
        public void CommandLine_Test122()
        {
            using (var vw = new VowpalWabbit("--confidence --confidence_after_training --initial_t 0.1 -d ./train-sets/rcv1_small.dat -p confidence_after_training.preds"))
            {
				                foreach (var dataLine in File.ReadLines(@"./train-sets/rcv1_small.dat"))
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/confidence_after_training.stderr", vw.PerformanceStatistics);
                							}
        }
        [TestMethod]
        [Description(@"recall tree hello world")]
        [TestCategory("Command Line")]
        public void CommandLine_Test126()
        {
            using (var vw = new VowpalWabbit("--quiet -d train-sets/gauss1k.dat.gz -f models/recall_tree_g100.model --recall_tree 100 -b 20 --loss_function logistic "))
            {
								using (var sr = new StreamReader(new GZipStream(new FileStream(@"train-sets/gauss1k.dat.gz", FileMode.Open), CompressionMode.Decompress)))
				{
				string dataLine;
				while ((dataLine = sr.ReadLine()) != null)
				                {
                    vw.Learn(dataLine);
                }
                                				 } 			}
        }
        [TestMethod]
        [Description(@"recall_tree hello world predict-from-saved-model")]
        [TestCategory("Command Line")]
        public void CommandLine_Test127()
        {
            using (var vw = new VowpalWabbit("--quiet -d train-sets/gauss1k.dat.gz -f models/recall_tree_g100.model --recall_tree 100 -b 20 --loss_function logistic "))
            {
								using (var sr = new StreamReader(new GZipStream(new FileStream(@"train-sets/gauss1k.dat.gz", FileMode.Open), CompressionMode.Decompress)))
				{
				string dataLine;
				while ((dataLine = sr.ReadLine()) != null)
				                {
                    vw.Learn(dataLine);
                }
                                				 } 			}
            using (var vw = new VowpalWabbit("-t -d train-sets/gauss1k.dat.gz -i models/recall_tree_g100.model"))
            {
								using (var sr = new StreamReader(new GZipStream(new FileStream(@"train-sets/gauss1k.dat.gz", FileMode.Open), CompressionMode.Decompress)))
				{
				string dataLine;
				while ((dataLine = sr.ReadLine()) != null)
				                {
                    vw.Learn(dataLine);
                }
                                                VWTestHelper.AssertEqual("train-sets/ref/recall_tree_gauss1k.stderr", vw.PerformanceStatistics);
                				 } 			}
        }
    }
}

