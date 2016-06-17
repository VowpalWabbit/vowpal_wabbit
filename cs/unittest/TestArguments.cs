using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class TestArgumentsClass
    {
        [TestMethod]
        public void TestArguments()
        {
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings("--cb_adf --rank_all --interact ud") { Verbose = true }))
            {
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--cb_adf"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--rank_all"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--interact ud"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_ldf multiline"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_rank"));
                vw.SaveModel("args.model");
            }

            using (var vw = new VowpalWabbit(new VowpalWabbitSettings { ModelStream = File.Open("args.model", FileMode.Open) }))
            {
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--no_stdin"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--max_prediction 1"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--bit_precision 18"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--cb_adf"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--cb_type ips"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--rank_all"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_ldf multiline"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--interact ud"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_rank"));
            }
        }
    }
}
