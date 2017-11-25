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
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test3()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k train-sets/0002.dat -f models/0002.model --invariant",
                @"train-sets\0002.dat",
                @"train-sets\ref\0002.stderr");
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
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
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
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
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        [Description("using normalized adaptive updates and a low --power_t")]
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
    }
}
