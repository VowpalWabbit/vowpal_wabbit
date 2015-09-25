
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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/0001.stderr", @"train-sets\ref")]

public void CommandLine_Test1() {

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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"test-sets/ref/0001.stderr", @"test-sets\ref")]

public void CommandLine_Test2() {

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
[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/0002.stderr", @"train-sets\ref")]

public void CommandLine_Test3() {

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
[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/0002.stderr", @"train-sets\ref")]

public void CommandLine_Test4() {

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
[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/0002a.stderr", @"train-sets\ref")]

public void CommandLine_Test5() {

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
[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/0002.stderr", @"train-sets\ref")]

[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"test-sets/ref/0002b.stderr", @"test-sets\ref")]

public void CommandLine_Test6() {

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
[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/0002c.stderr", @"train-sets\ref")]

public void CommandLine_Test7() {

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
[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/0002c.stderr", @"train-sets\ref")]

[DeploymentItem(@"train-sets/0002.dat",@"train-sets")]
[DeploymentItem(@"test-sets/ref/0002c.stderr", @"test-sets\ref")]

public void CommandLine_Test8() {

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
// Skipping test, unable to parse prediction file
// Skipping test, unable to parse prediction file

		[TestMethod]
		[Description("one-against-all")]
		[TestCategory("Command line")]
[DeploymentItem(@"train-sets/multiclass",@"train-sets")]
[DeploymentItem(@"train-sets/ref/oaa.stderr", @"train-sets\ref")]

public void CommandLine_Test11() {

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
[DeploymentItem(@"train-sets/multiclass",@"train-sets")]
[DeploymentItem(@"train-sets/ref/multiclass.stderr", @"train-sets\ref")]

public void CommandLine_Test12() {

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
[DeploymentItem(@"train-sets/zero.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/zero.stderr", @"train-sets\ref")]

public void CommandLine_Test15() {

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
[DeploymentItem(@"train-sets/xxor.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/xxor.stderr", @"train-sets\ref")]

public void CommandLine_Test21() {

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
[DeploymentItem(@"train-sets/ml100k_small_train",@"train-sets")]
[DeploymentItem(@"train-sets/ref/ml100k_small.stderr", @"train-sets\ref")]

public void CommandLine_Test22() {

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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/bs.vote.stderr", @"train-sets\ref")]

public void CommandLine_Test27() {

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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/bs.vote.stderr", @"train-sets\ref")]

[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/bs.prvote.stderr", @"train-sets\ref")]

public void CommandLine_Test28() {

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
[DeploymentItem(@"train-sets/affix_test.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/affix_test.stderr", @"train-sets\ref")]

public void CommandLine_Test29() {

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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/mask.stderr", @"train-sets\ref")]

public void CommandLine_Test30() {

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
// Skipping test, unable to parse prediction file

		[TestMethod]
		[Description("non-centered data-set where constant >> 0")]
		[TestCategory("Command line")]
[DeploymentItem(@"train-sets/big-constant.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/big-constant.stderr", @"train-sets\ref")]

public void CommandLine_Test35() {

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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/progress-10.stderr", @"train-sets\ref")]

public void CommandLine_Test36() {

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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/progress-0.5.stderr", @"train-sets\ref")]

public void CommandLine_Test37() {

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
[DeploymentItem(@"train-sets/0001.dat",@"train-sets")]
[DeploymentItem(@"train-sets/ref/nn-1-noquiet.stderr", @"train-sets\ref")]

public void CommandLine_Test38() {

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
[DeploymentItem(@"train-sets/rcv1_raw_cb_small.vw",@"train-sets")]
[DeploymentItem(@"train-sets/ref/rcv1_raw_cb_dr.stderr", @"train-sets\ref")]

public void CommandLine_Test39() {

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
[DeploymentItem(@"train-sets/rcv1_raw_cb_small.vw",@"train-sets")]
[DeploymentItem(@"train-sets/ref/rcv1_raw_cb_ips.stderr", @"train-sets\ref")]

public void CommandLine_Test40() {

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
} }
