using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{

    [TestClass]
    public class RunTests : TestBase
    {
        [TestMethod]
        [DeploymentItem("RunTests", "RunTests")]
        [DeploymentItem("train-sets", "train-sets")]
        [DeploymentItem(@"train-sets\ref", @"train-sets\ref")]
        [DeploymentItem(@"pred-sets\ref", @"pred-sets\ref")]
        public void RunAllTest()
        {
            var runTests = File.ReadAllLines("RunTests")
                .SkipWhile(l => l != "__DATA__");
        }
    }
}
