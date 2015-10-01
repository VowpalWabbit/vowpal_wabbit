
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using VW;

namespace cs_unittest
{
    [TestClass]
    public partial class TestAll
    {


        [TestMethod]
        [Description("")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0001.stderr", @"train-sets\ref")]

        public void CommandLine_Test1()
        {

            using (var vw = new VowpalWabbit("-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0001.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("checking predictions as well")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0001.stderr", @"train-sets\ref")]

        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"test-sets/ref/0001.stderr", @"test-sets\ref")]

        public void CommandLine_Test2()
        {

            using (var vw = new VowpalWabbit("-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0001.stderr", vw.PerformanceStatistics);
            }


            using (var vw = new VowpalWabbit("-k -t train-sets/0001.dat -i models/0001.model -p 0001.predict --invariant"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("test-sets/ref/0001.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("without -d, training only")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0002.stderr", @"train-sets\ref")]

        public void CommandLine_Test3()
        {

            using (var vw = new VowpalWabbit("-k train-sets/0002.dat -f models/0002.model --invariant"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0002.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("same, with -d")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0002.stderr", @"train-sets\ref")]

        public void CommandLine_Test4()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0002.dat -f models/0002.model --invariant"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0002.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("add -q .., adaptive, and more (same input, different outputs)")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0002a.stderr", @"train-sets\ref")]

        public void CommandLine_Test5()
        {

            using (var vw = new VowpalWabbit("-k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model train-sets/0002.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0002a.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("run predictions on Test 4 model")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0002.stderr", @"train-sets\ref")]

        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"test-sets/ref/0002b.stderr", @"test-sets\ref")]

        public void CommandLine_Test6()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0002.dat -f models/0002.model --invariant"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0002.stderr", vw.PerformanceStatistics);
            }


            using (var vw = new VowpalWabbit("-k -t -i models/0002.model -d train-sets/0002.dat -p 0002b.predict"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("test-sets/ref/0002b.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("using normalized adaptive updates and a low --power_t")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0002c.stderr", @"train-sets\ref")]

        public void CommandLine_Test7()
        {

            using (var vw = new VowpalWabbit("-k --power_t 0.45 -f models/0002c.model train-sets/0002.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0002c.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("predicts on test 7 model")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0002c.stderr", @"train-sets\ref")]

        [DeploymentItem(@"train-sets/0002.dat", @"train-sets")]
        [DeploymentItem(@"test-sets/ref/0002c.stderr", @"test-sets\ref")]

        public void CommandLine_Test8()
        {

            using (var vw = new VowpalWabbit("-k --power_t 0.45 -f models/0002c.model train-sets/0002.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0002c.stderr", vw.PerformanceStatistics);
            }


            using (var vw = new VowpalWabbit("-k -t -i models/0002c.model -d train-sets/0002.dat -p 0002c.predict"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0002.dat"))
                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("test-sets/ref/0002c.stderr", vw.PerformanceStatistics);
            }

        }
        /* Unable to parse command line: lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload. */
        /* Unable to parse command line: lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload. */

        [TestMethod]
        [Description("one-against-all")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/multiclass", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/oaa.stderr", @"train-sets\ref")]

        public void CommandLine_Test11()
        {

            using (var vw = new VowpalWabbit("-k --oaa 10 -c --passes 10 train-sets/multiclass --holdout_off"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/multiclass"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/oaa.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("Error Correcting Tournament")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/multiclass", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/multiclass.stderr", @"train-sets\ref")]

        public void CommandLine_Test12()
        {

            using (var vw = new VowpalWabbit("-k --ect 10 --error 3 -c --passes 10 --invariant train-sets/multiclass --holdout_off"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/multiclass"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/multiclass.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("LBFGS on zero derivative input")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/zero.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/zero.stderr", @"train-sets\ref")]

        public void CommandLine_Test15()
        {

            using (var vw = new VowpalWabbit("-k -c -d train-sets/zero.dat --loss_function=squared -b 20 --bfgs --mem 7 --passes 5 --l2 1.0 --holdout_off"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/zero.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/zero.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cubic features -- on a parity test case")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/xxor.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/xxor.stderr", @"train-sets\ref")]

        public void CommandLine_Test21()
        {

            using (var vw = new VowpalWabbit("-k -c -f models/xxor.model train-sets/xxor.dat --cubic abc --passes 100 --holdout_off --progress 1.33333"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/xxor.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/xxor.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("matrix factorization -- training")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/ml100k_small_train", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/ml100k_small.stderr", @"train-sets\ref")]

        public void CommandLine_Test22()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/ml100k_small_train -b 16 -q ui --rank 10 --l2 2e-6 --learning_rate 0.05 --passes 2 --decay_learning_rate 0.97 --power_t 0 -f models/movielens.reg -c --loss_function classic --holdout_off"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/ml100k_small_train"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/ml100k_small.stderr", vw.PerformanceStatistics);
            }

        }
        // Skipping inconsistent test -t without .predict file

        [TestMethod]
        [Description("bagging -- binary classifiers")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/bs.vote.stderr", @"train-sets\ref")]

        public void CommandLine_Test27()
        {

            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/bs.vote.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("bagging -- predict with bagged classifier")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/bs.vote.stderr", @"train-sets\ref")]

        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/bs.prvote.stderr", @"train-sets\ref")]

        public void CommandLine_Test28()
        {

            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -f models/bs.vote.model --bootstrap 4 --bs_type vote -p bs.vote.predict"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/bs.vote.stderr", vw.PerformanceStatistics);
            }


            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -i models/bs.vote.model -p bs.prvote.predict -t"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/bs.prvote.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("affix features")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/affix_test.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/affix_test.stderr", @"train-sets\ref")]

        public void CommandLine_Test29()
        {

            using (var vw = new VowpalWabbit("-d train-sets/affix_test.dat -k -c --passes 10 --holdout_off --affix -2"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/affix_test.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/affix_test.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("train --l1 regularized model")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/mask.stderr", @"train-sets\ref")]

        public void CommandLine_Test30()
        {

            using (var vw = new VowpalWabbit("-d train-sets/0001.dat -f models/mask.model --invert_hash mask.predict --l1 0.01"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/mask.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("non-centered data-set where constant >> 0")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/big-constant.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/big-constant.stderr", @"train-sets\ref")]

        public void CommandLine_Test35()
        {

            using (var vw = new VowpalWabbit("-k --passes 100 -c --holdout_off --constant 1000 -d train-sets/big-constant.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/big-constant.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/big-constant.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("new option: --progress w/ integer arg")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/progress-10.stderr", @"train-sets\ref")]

        public void CommandLine_Test36()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat --progress 10"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/progress-10.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("new-option: --progress w/ floating-point arg")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/progress-0.5.stderr", @"train-sets\ref")]

        public void CommandLine_Test37()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -P 0.5"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/progress-0.5.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("--nn without --quiet to avoid nn regressions")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/nn-1-noquiet.stderr", @"train-sets\ref")]

        public void CommandLine_Test38()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat --nn 1"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/nn-1-noquiet.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cb with dr")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/rcv1_raw_cb_small.vw", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/rcv1_raw_cb_dr.stderr", @"train-sets\ref")]

        public void CommandLine_Test39()
        {

            using (var vw = new VowpalWabbit("-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dr --ngram 2 --skips 4 -b 24 -l 0.25"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/rcv1_raw_cb_small.vw"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/rcv1_raw_cb_dr.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cb with ips")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/rcv1_raw_cb_small.vw", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/rcv1_raw_cb_ips.stderr", @"train-sets\ref")]

        public void CommandLine_Test40()
        {

            using (var vw = new VowpalWabbit("-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type ips --ngram 2 --skips 4 -b 24 -l 0.125"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/rcv1_raw_cb_small.vw"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/rcv1_raw_cb_ips.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cb with dm")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/rcv1_raw_cb_small.vw", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/rcv1_raw_cb_dm.stderr", @"train-sets\ref")]

        public void CommandLine_Test41()
        {

            using (var vw = new VowpalWabbit("-d train-sets/rcv1_raw_cb_small.vw --cb 2 --cb_type dm --ngram 2 --skips 4 -b 24 -l 0.125"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/rcv1_raw_cb_small.vw"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/rcv1_raw_cb_dm.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("SVM linear kernel")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/rcv1_smaller.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/ksvm_train.linear.stderr", @"train-sets\ref")]

        public void CommandLine_Test62()
        {

            using (var vw = new VowpalWabbit("--ksvm --l2 1 --reprocess 5 -b 18 -p ksvm_train.linear.predict -d train-sets/rcv1_smaller.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/rcv1_smaller.dat"))
                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/ksvm_train.linear.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("SVM polynomial kernel")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/rcv1_smaller.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/ksvm_train.poly.stderr", @"train-sets\ref")]

        public void CommandLine_Test63()
        {

            using (var vw = new VowpalWabbit("--ksvm --l2 1 --reprocess 5 -b 18 --kernel poly -p ksvm_train.poly.predict -d train-sets/rcv1_smaller.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/rcv1_smaller.dat"))
                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/ksvm_train.poly.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("SVM rbf kernel")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/rcv1_smaller.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/ksvm_train.rbf.stderr", @"train-sets\ref")]

        public void CommandLine_Test64()
        {

            using (var vw = new VowpalWabbit("--ksvm --l2 1 --reprocess 5 -b 18 --kernel rbf -p ksvm_train.rbf.predict -d train-sets/rcv1_smaller.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/rcv1_smaller.dat"))
                {
                    var actualValue = vw.Learn(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/ksvm_train.rbf.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("train FTRL-Proximal")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0001_ftrl.stderr", @"train-sets\ref")]

        public void CommandLine_Test72()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/0001_ftrl.model --passes 1 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0001_ftrl.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("test FTRL-Proximal")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"test-sets/ref/0001_ftrl.stderr", @"test-sets\ref")]

        public void CommandLine_Test73()
        {

            using (var vw = new VowpalWabbit("-k -t train-sets/0001.dat -i models/0001_ftrl.model -p 0001_ftrl.predict"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    var actualValue = vw.Predict(dataLine, VowpalWabbitPredictionType.Scalar);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("test-sets/ref/0001_ftrl.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cb evaluation")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/rcv1_cb_eval", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/rcv1_cb_eval.stderr", @"train-sets\ref")]

        public void CommandLine_Test74()
        {

            using (var vw = new VowpalWabbit("-d train-sets/rcv1_cb_eval --cb 2 --eval"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/rcv1_cb_eval"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/rcv1_cb_eval.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("Log_multi")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/multiclass", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/log_multi.stderr", @"train-sets\ref")]

        public void CommandLine_Test75()
        {

            using (var vw = new VowpalWabbit("--log_multi 10 train-sets/multiclass "))
            {
                foreach (var dataLine in File.ReadLines("train-sets/multiclass"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/log_multi.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cbify, epsilon-greedy")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/multiclass", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/cbify_epsilon.stderr", @"train-sets\ref")]

        public void CommandLine_Test76()
        {

            using (var vw = new VowpalWabbit("--cbify 10 --epsilon 0.05 train-sets/multiclass "))
            {
                foreach (var dataLine in File.ReadLines("train-sets/multiclass"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/cbify_epsilon.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cbify, bag")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/multiclass", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/cbify_bag.stderr", @"train-sets\ref")]

        public void CommandLine_Test78()
        {

            using (var vw = new VowpalWabbit("--cbify 10 --bag 7 train-sets/multiclass "))
            {
                foreach (var dataLine in File.ReadLines("train-sets/multiclass"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/cbify_bag.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("cbify, cover")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/multiclass", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/cbify_cover.stderr", @"train-sets\ref")]

        public void CommandLine_Test79()
        {

            using (var vw = new VowpalWabbit("--cbify 10 --cover 3 train-sets/multiclass "))
            {
                foreach (var dataLine in File.ReadLines("train-sets/multiclass"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/cbify_cover.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("lrq empty namespace")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0080.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0080.stderr", @"train-sets\ref")]

        public void CommandLine_Test80()
        {

            using (var vw = new VowpalWabbit("--lrq aa3 -d train-sets/0080.dat"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0080.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0080.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("train FTRL-PiSTOL")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/ftrl_pistol.stderr", @"train-sets\ref")]

        public void CommandLine_Test81()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0001.dat -f models/ftrl_pistol.model --passes 1 --pistol"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/ftrl_pistol.stderr", vw.PerformanceStatistics);
            }

        }
        // Skipping inconsistent test -t without .predict file

        [TestMethod]
        [Description("check redefine functionality")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0080.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/redefine.stderr", @"train-sets\ref")]

        public void CommandLine_Test83()
        {

            using (var vw = new VowpalWabbit("-k -d train-sets/0080.dat --redefine := --redefine y:=: --redefine x:=arma --ignore x -q yy"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0080.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/redefine.stderr", vw.PerformanceStatistics);
            }

        }
        /* Unable to parse command line: lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload. */
        // Skipping test, unable to parse prediction file
        /* Unable to parse command line: lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload. */
        /* Unable to parse command line: lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload. */

        [TestMethod]
        [Description("named labels at training time")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/test_named", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/test_named_train.stderr", @"train-sets\ref")]

        public void CommandLine_Test88()
        {

            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --oaa 3 -d train-sets/test_named  -k -c --passes 10 --holdout_off -f models/test_named.model"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/test_named"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/test_named_train.stderr", vw.PerformanceStatistics);
            }

        }
        // Skipping test, unable to parse prediction file

        [TestMethod]
        [Description("named labels at training time (csoaa)")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/test_named_csoaa", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/test_named_csoaa_train.stderr", @"train-sets\ref")]

        public void CommandLine_Test90()
        {

            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa  -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/test_named_csoaa"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/test_named_csoaa_train.stderr", vw.PerformanceStatistics);
            }

        }
        // Skipping test, unable to parse prediction file
        /* Unable to parse command line: Empty path name is not legal.
        Parameter name: path */
        /* Unable to parse command line: lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload. */

        [TestMethod]
        [Description("experience replay version of test 1")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/0001.dat", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/0001-replay.stderr", @"train-sets\ref")]

        public void CommandLine_Test94()
        {

            using (var vw = new VowpalWabbit("-k -l 20 --initial_t 128000 --power_t 1 -d train-sets/0001.dat -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off --replay_b 100"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/0001.dat"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/0001-replay.stderr", vw.PerformanceStatistics);
            }

        }

        [TestMethod]
        [Description("named labels at training time (csoaa) with experience replay")]
        [TestCategory("Command line")]
        [DeploymentItem(@"train-sets/test_named_csoaa", @"train-sets")]
        [DeploymentItem(@"train-sets/ref/test_named_csoaa_train-replay.stderr", @"train-sets\ref")]

        public void CommandLine_Test95()
        {

            using (var vw = new VowpalWabbit("--named_labels det,noun,verb --csoaa 3 -d train-sets/test_named_csoaa -k -c --passes 10 --holdout_off -f models/test_named_csoaa.model --replay_c 100"))
            {
                foreach (var dataLine in File.ReadLines("train-sets/test_named_csoaa"))
                {
                    vw.Learn(dataLine);
                }
                vw.RunMultiPass();
                VWTestHelper.AssertEqual("train-sets/ref/test_named_csoaa_train-replay.stderr", vw.PerformanceStatistics);
            }

        }
        /* Unable to parse command line: Empty path name is not legal.
        Parameter name: path */
    }
}
