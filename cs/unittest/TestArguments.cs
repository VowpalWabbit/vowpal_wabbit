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
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings("--cb_explore_adf --epsilon 0.3 --interact ud") { Verbose = true }))
            {
                Assert.AreEqual("--cb_explore_adf --epsilon 0.3 --interact ud --cb_adf --csoaa_ldf multiline --csoaa_rank", vw.Arguments.CommandLine);

                vw.SaveModel("args.model");
            }

            using (var vw = new VowpalWabbit(new VowpalWabbitSettings { ModelStream = File.Open("args.model", FileMode.Open) }))
            {
                Assert.AreEqual("--no_stdin --max_prediction 1 --bit_precision 18 --cb_explore_adf --cb_adf --cb_type ips --csoaa_ldf multiline --interact ud",
                    vw.Arguments.CommandLine);
            }
        }
    }
}
