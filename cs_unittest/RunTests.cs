using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
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
//            var runTests = File.ReadAllText("RunTests");

//            var match = Regex.Match(runTests, "__DATA__(.*)$", RegexOptions.Multiline);

//            Assert.IsTrue(match.Success,"RunTests format changed");

//            runTests = match.Groups[1].Value;

//            var matches = Regex.Matches(runTests, @"
//                    # Test (?<nr>\d+) : (?<comment>.*\\n)?
//                    {VW}
//
//                    ", RegexOptions.Multiline | RegexOptions.IgnorePatternWhitespace)

        }
    }
}
