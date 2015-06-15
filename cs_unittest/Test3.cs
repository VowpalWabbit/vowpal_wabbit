using Antlr4.Runtime;
using Antlr4.Runtime.Tree;
using cs_test;
using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Interfaces;
using Microsoft.Research.MachineLearning.Labels;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
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
            using (var vw = new VowpalWabbit<Data>("-k train-sets/0002.dat -f models/0002.model --invariant"))
            {
                VWTestHelper.ParseInput(
                    File.OpenRead(@"train-sets\0002.dat"),
                    new DataListener(x => 
                        {
                            using (var ex = vw.ReadExample(x))
                            {
                                ex.Learn<VowpalWabbitPredictionNone>();
                            }
                        }));
            
                VWTestHelper.AssertEqual(@"train-sets\ref\0002.stderr", vw.PerformanceStatistics);
            }
        }

        [TestMethod]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002.stderr", @"train-sets\ref")]
        public void Test4()
        {
            using (var vw = new VowpalWabbit<Data>("-k -d train-sets/0002.dat -f models/0002.model --invariant"))
            {
                VWTestHelper.ParseInput(
                    File.OpenRead(@"train-sets\0002.dat"),
                    new DataListener(x =>
                    {
                        using (var ex = vw.ReadExample(x))
                        {
                            ex.Learn<VowpalWabbitPredictionNone>();
                        }
                    }));

                VWTestHelper.AssertEqual(@"train-sets\ref\0002.stderr", vw.PerformanceStatistics);
            }
        }

        [TestMethod]
        [DeploymentItem(@"train-sets\0002.dat", "train-sets")]
        [DeploymentItem(@"train-sets\ref\0002a.stderr", @"train-sets\ref")]
        // pred-sets/ref/0002b.predict
        public void Test5()
        {
            using (var vw = new VowpalWabbit<Data>("-k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model train-sets/0002.dat"))
            {
                VWTestHelper.ParseInput(
                    File.OpenRead(@"train-sets\0002.dat"),
                    new DataListener(x =>
                    {
                        using (var ex = vw.ReadExample(x))
                        {
                            ex.Learn<VowpalWabbitPredictionNone>();
                        }
                    }));

                VWTestHelper.AssertEqual(@"train-sets\ref\0002a.stderr", vw.PerformanceStatistics);
            }

            var references = File.ReadAllLines(@"pred-sets\ref\0001.predict").Select(l => float.Parse(l, CultureInfo.InvariantCulture)).ToArray();


            using (var vwRef = new VowpalWabbit("-k -t --invariant -i models/0002a.model"))
            using (var vwModel = new VowpalWabbitModel("-k -t --invariant", File.OpenRead("models/0002a.model")))
            using (var vwInMemoryShared2 = new VowpalWabbit<Data>(vwModel))
            {
                VWTestHelper.ParseInput(
                    File.OpenRead(@"train-sets\0002.dat"),
                    new DataListener(x =>
                    {
                        var expected = vwRef.Predict<VowpalWabbitScalarPrediction>(x.Line);

                        using (var ex = vwInMemoryShared2.ReadExample(x))
                        {
                            var actual = ex.Predict<VowpalWabbitScalarPrediction>();

                            Assert.AreEqual(expected.Value, actual.Value, 1e-5);
                        }
                    }));
            }
        }
    }
}
