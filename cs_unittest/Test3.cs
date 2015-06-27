using Antlr4.Runtime;
using Antlr4.Runtime.Tree;
using cs_test;
using VW;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TrainSet0002Dat;

namespace cs_unittest
{
    [TestClass]
    public class Test3Class : TestBase
    {
        [TestMethod]
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
            using (var vw = new VowpalWabbit<Cs_TestData>("-k -c -p cs_test.ldf.csoaa.predict --passes 10 --invariant --csoaa_ldf multiline --holdout_off"))
            {
                foreach (var d in sampleData)
	            {
                    using (var ex = vw.ReadExample(d))
                    {
                        ex.Learn();
                    }		 
	            }

                vw.RunMultiPass();

                VWTestHelper.AssertEqual(@"train-sets\ref\cs_test.ldf.csoaa.stderr", vw.PerformanceStatistics);
            }

            Assert.AreEqual(
                File.ReadAllText(@"train-sets\ref\cs_test.ldf.csoaa.predict"),
                File.ReadAllText("cs_test.ldf.csoaa.predict"));
        }
    }
}
