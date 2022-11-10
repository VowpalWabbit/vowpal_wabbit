using System.Diagnostics;
using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using TrainSet0002Dat;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class BuilderTestClass : TestBase
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/example builder")]
        public void TestBuilderSimple()
        {
          using (VowpalWabbit vw = new VowpalWabbit(""))
          {
            VowpalWabbitExample e;

            using (var exampleBuilder = new VowpalWabbitExampleBuilder(vw))
            using (var nsBuilder = exampleBuilder.AddNamespace('U'))
            {
                ulong nsHash = vw.HashSpace("User");
                nsBuilder.AddFeature(vw.HashFeature("e1", nsHash), 0.3425f);

                e = exampleBuilder.CreateExample();
            }

            Debug.Assert(e != null);
            foreach (var n in e)
            {
                Debug.WriteLine($"+ ({n.Index})=>'{(char)n.Index}'");
                foreach (var f in n)
                {
                    Debug.WriteLine($"-- {f.WeightIndex}:{f.X}");
                }
            }
          }
        }
    }

    [TestClass]
    public class Test3Class : TestBase
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test3()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k train-sets/0002.dat -f models/0002.model --invariant",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002.stderr");
#endif
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test4and6()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k -d train-sets/0002.dat -f models/0002.model --invariant",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002.stderr");
#endif

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t --invariant -i models/0002.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("pred-sets", "ref", "0002b.predict"));
#else
                @"train-sets\0002.dat",
                @"pred-sets\ref\0002b.predict");
#endif
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test5()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002a.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002a.stderr");
#endif

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t --invariant -i models/0002a.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"));
#else
                @"train-sets\0002.dat");
#endif
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        [Description("using normalized adaptive updates and a low --power_t")]
        public void Test7and8()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k --power_t 0.45 -f models/0002c.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002c.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002c.stderr");
#endif

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t -i models/0002c.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("pred-sets", "ref", "0002c.predict"));
#else
                @"train-sets\0002.dat",
                @"pred-sets\ref\0002c.predict");
#endif
        }
    }
}
