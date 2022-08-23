using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class TestArgumentsClass
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestArguments()
        {
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings("--cb_explore_adf --epsilon 0.3 --interact ud") { Verbose = true }))
            {
                // --cb_explore_adf --epsilon 0.3 --interact ud --cb_adf --csoaa_ldf multiline --csoaa_rank
                Console.WriteLine(vw.Arguments.CommandLine);
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--cb_explore_adf"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--epsilon 0.3"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--interact ud"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_ldf multiline"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_rank"));
                vw.SaveModel("args.model");
            }

            using (var vw = new VowpalWabbit(new VowpalWabbitSettings { ModelStream = File.Open("args.model", FileMode.Open) }))
            {
                Console.WriteLine(vw.Arguments.CommandLine);
                // --no_stdin --bit_precision 18 --cb_explore_adf --epsilon 0.300000 --cb_adf --cb_type mtr --csoaa_ldf multiline --csoaa_rank --interact ud

                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--no_stdin"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--bit_precision 18"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--cb_explore_adf"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--epsilon 0.3"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--interact ud"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_ldf multiline"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--csoaa_rank"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("--cb_type mtr"));
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestQuietAndTestArguments()
        {
            using (var vw = new VowpalWabbit("--quiet -t"))
            {
                vw.SaveModel("args.model");
            }

            using (var vw = new VowpalWabbitModel(new VowpalWabbitSettings { ModelStream = File.Open("args.model", FileMode.Open) }))
            {
                Assert.IsFalse(vw.Arguments.CommandLine.Contains("--quiet"));

                // At first glance it seems like this should be IsFalse, given that VW is not meant to persiste
                // the -t flag in the model file. What is going on, however is that when we use VowpalWabbitModel
                // as the mechanism to load a VW model file (for the purposes of seeding other instances with
                // the weights, it must be made TestOnly. This checks to ensure that the way that VowpalWabbitModel
                // works is correct, not the file persistence.
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("-t"));

                using (var vwSub = new VowpalWabbit(new VowpalWabbitSettings { Model = vw }))
                {
                    Assert.IsTrue(vwSub.Arguments.CommandLine.Contains("--quiet"));
                    Assert.IsTrue(vwSub.Arguments.CommandLine.Contains("-t"));
                }
            }

            using (var vw = new VowpalWabbit(""))
            {
                vw.SaveModel("args.model");
            }

            using (var vw = new VowpalWabbitModel(new VowpalWabbitSettings { ModelStream = File.Open("args.model", FileMode.Open) }))
            {
                Assert.IsFalse(vw.Arguments.CommandLine.Contains("--quiet"));
                Assert.IsTrue(vw.Arguments.CommandLine.Contains("-t"));

                using (var vwSub = new VowpalWabbit(new VowpalWabbitSettings { Model = vw }))
                {
                    Assert.IsTrue(vwSub.Arguments.CommandLine.Contains("--quiet"));
                    Assert.IsTrue(vwSub.Arguments.CommandLine.Contains("-t"));
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestArgumentDeDup()
        {
            using (var vw = new VowpalWabbit("-l 0.3 -l 0.3 --learning_rate 0.3 -f model1 -q ab"))
            {
                Assert.AreEqual(0.3f, vw.Native.Arguments.LearningRate);
            }

            try
            {
                using (var vw =
                    new VowpalWabbit(
                        "--cb 2 --cb_type ips --cb_type dm --learning_rate 0.1 -f model_bad -q ab"))
                {
                    Assert.AreEqual(0.1f, vw.Native.Arguments.LearningRate);
                }

                Assert.Fail("Disagreering arguments not detected");
            }
            catch (VowpalWabbitException)
            { }

            using (var vw = new VowpalWabbit("-i model1"))
            {
                Assert.AreEqual(0.3f, vw.Native.Arguments.LearningRate);
            }

            using (var vw = new VowpalWabbit("-i model1 -q ab -l 0.4"))
            {
                Assert.AreEqual(0.4f, vw.Native.Arguments.LearningRate);
            }

            // make sure different representations of arguments are matched
            using (var vw = new VowpalWabbit("--cb_explore_adf --epsilon 0.1 -f model2"))
            { }

            using (var vw = new VowpalWabbit("--cb_explore_adf --epsilon 0.1000 -i model2"))
            { }
        }
    }
}
