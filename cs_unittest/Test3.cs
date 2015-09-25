using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using TrainSet0002Dat;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class Test3Class : TestBase
    {
        [TestMethod]
        [TestCategory("Command line through marshalling")]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002.stderr", @"train-sets\ref")]
        public void Test3()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k train-sets/0002.dat -f models/0002.model --invariant",
                @"train-sets\0002.dat",
                @"train-sets\ref\0002.stderr");
        }

        [TestMethod]
        [TestCategory("Command line through marshalling")]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002.stderr", @"train-sets\ref")]
        [DeploymentItem(@"pred-sets\ref\0002b.predict", @"pred-sets\ref")]
        public void Test4and6()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k -d train-sets/0002.dat -f models/0002.model --invariant",
                @"train-sets\0002.dat",
                @"train-sets\ref\0002.stderr");

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t --invariant -i models/0002.model",
                @"train-sets\0002.dat",
                @"pred-sets\ref\0002b.predict");
        }

        [TestMethod]
        [TestCategory("Command line through marshalling")]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002a.stderr", @"train-sets\ref")]
        public void Test5()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model",
                @"train-sets\0002.dat",
                @"train-sets\ref\0002a.stderr");

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t --invariant -i models/0002a.model",
                @"train-sets\0002.dat");
        }

        [TestMethod]
        [TestCategory("Command line through marshalling")]
        [Description("using normalized adaptive updates and a low --power_t")]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002c.stderr", @"train-sets\ref")]
        [DeploymentItem(@"pred-sets\ref\0002c.predict", @"pred-sets\ref")]
        public void Test7and8()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k --power_t 0.45 -f models/0002c.model",
                @"train-sets\0002.dat",
                @"train-sets\ref\0002c.stderr");

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t -i models/0002c.model",
                @"train-sets\0002.dat",
                @"pred-sets\ref\0002c.predict");
        }

        [TestMethod]
        [Ignore]
        [Description("label-dependent features with csoaa_ldf")]
        [DeploymentItem(@"train-sets\ref\cs_test.ldf.csoaa.stderr", @"train-sets\ref")]
        [DeploymentItem(@"train-sets\ref\cs_test.ldf.csoaa.predict", @"train-sets\ref")]
        public void Test9()
        {
            var sampleData = TrainSetCs_testLdf.CreateSampleCbAdfData();

            //            # Test 9: label-dependent features with csoaa_ldf
            //{VW} -k -c -d train-sets/cs_test.ldf -p cs_test.ldf.csoaa.predict --passes 10 --invariant --csoaa_ldf multiline --holdout_off
            //    train-sets/ref/cs_test.ldf.csoaa.stderr
            //    train-sets/ref/cs_test.ldf.csoaa.predict
            using (var vw = new VowpalWabbit<Cs_TestData, Cs_TestCs_TestDataADF>("-k -c -p cs_test.ldf.csoaa.predict --passes 10 --invariant --csoaa_ldf multiline --holdout_off"))
            {
                foreach (var d in sampleData)
                {
                    var index = d.ActionDependentFeatures.IndexOf(a => a.Label != null);
                    var label = d.ActionDependentFeatures[index].Label;

                    vw.Learn(d, d.ActionDependentFeatures, index, label);
                }

                vw.Native.RunMultiPass();

                VWTestHelper.AssertEqual(@"train-sets\ref\cs_test.ldf.csoaa.stderr", vw.Native.PerformanceStatistics);
            }

            Assert.AreEqual(
                File.ReadAllText(@"train-sets\ref\cs_test.ldf.csoaa.predict"),
                File.ReadAllText("cs_test.ldf.csoaa.predict"));
        }
    }
}
