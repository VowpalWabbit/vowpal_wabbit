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
    public class TestModelLoading
    {
        [TestMethod]
        [DeploymentItem(@"model-sets\7.10.2_corrupted.model", "model-sets")]
        [DeploymentItem(@"model-sets\8.0.0_ok.model", "model-sets")]
        [DeploymentItem(@"model-sets\8.0.1_hash_ok.model", "model-sets")]
        public void TestLoadModel()
        {
            InternalTestModel(@"model-sets/7.10.2_corrupted.model", false);
            InternalTestModel(@"model-sets/8.0.0_ok.model", true);
            InternalTestModel(@"model-sets/8.0.1_hash_ok.model", true);
        }

        [TestMethod]
        [DeploymentItem(@"model-sets\8.0.1_rcv1_ok.model", "model-sets")]
        [DeploymentItem(@"model-sets\8.0.1.test_named_ok.model", "model-sets")]
        [DeploymentItem(@"model-sets\8.0.1_hash_ok.model", "model-sets")]
        public void TestLoadModelRandomCorrupt()
        {
            InternalTestModelRandomCorrupt("model-sets/8.0.1.test_named_ok.model");
            InternalTestModelRandomCorrupt("model-sets/8.0.1_rcv1_ok.model");
            InternalTestModelRandomCorrupt("model-sets/8.0.1_hash_ok.model");
        }

        private void InternalTestModel(string modelFile, bool shouldPass)
        {
            try
            {
                using (var vw = new VowpalWabbitModel(string.Format("--quiet -t -i {0}", modelFile)))
                {
                    // should only reach this point if model is valid
                    Assert.IsTrue(shouldPass);
                }
            }
            catch (VowpalWabbitException ex)
            {
                Assert.IsTrue(ex.Message.Contains("corrupted"));
            }
        }

        private void InternalTestModelRandomCorrupt(string modelFile)
        {
            const int numBytesToCorrupt = 10;

            var rand = new Random(0);
            byte[] modelBytes = File.ReadAllBytes(modelFile);

            for (int i = 0; i < 100; i++)
            {
                var corruptBytes = new byte[modelBytes.Length];
                Array.Copy(modelBytes, corruptBytes, corruptBytes.Length);

                for (int j = 0; j < numBytesToCorrupt; j++)
                {
                    corruptBytes[rand.Next(corruptBytes.Length)] = (byte)rand.Next(byte.MaxValue);
                }

                try
                {
                    using (var modelStream = new MemoryStream(corruptBytes))
                    using (var vw = new VowpalWabbitModel(new VowpalWabbitSettings("--quiet -t", modelStream)))
                    {
                        // chances of reaching this point after reading a corrupt model are low
                        Assert.IsTrue(false);
                    }
                }
                catch (Exception) // an exception should be caught unless AV is encountered in which case the test will fail
                {
                    Assert.IsTrue(true);
                }
            }
        }
    }

    
}
