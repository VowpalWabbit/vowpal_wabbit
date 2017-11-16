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
    public class TestModelLoading : TestBase
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Model Loading")]
        public void TestLoadModelCorrupt()
        {
            InternalTestModel(@"model-sets/7.10.2_corrupted.model", false);
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Model Loading")]
        public void TestLoadModel()
        {
            InternalTestModel(@"model-sets/8.0.0_ok.model", true);
            InternalTestModel(@"model-sets/8.0.1.test_named_ok.model", true);
            InternalTestModel(@"model-sets/8.0.1_rcv1_ok.model", true);
            InternalTestModel(@"model-sets/8.0.1_hash_ok.model", true);
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Model Loading")]
        public void TestLoadModelRandomCorrupt()
        {
            InternalTestModelRandomCorrupt("model-sets/8.0.1.test_named_ok.model");
            //InternalTestModelRandomCorrupt("model-sets/8.0.1_rcv1_ok.model");
            //InternalTestModelRandomCorrupt("model-sets/8.0.1_hash_ok.model");
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Model Loading")]
        public void TestLoadModelInMemory()
        {
            using (var vw = new VowpalWabbit(@"-i model-sets\8.0.1_rcv1_ok.model"))
            {
                var memStream = new MemoryStream();
                vw.SaveModel(memStream);

                vw.SaveModel("native.model");

                using (var file = File.Create("managed.file.model"))
                {
                    vw.SaveModel(file);
                }

                var nativeModel = File.ReadAllBytes("native.model");
                var managedFileModel = File.ReadAllBytes("managed.file.model");
                var managedModel = memStream.ToArray();

                Assert.IsTrue(nativeModel.SequenceEqual(managedModel));
                Assert.IsTrue(nativeModel.SequenceEqual(managedFileModel));
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Model Loading")]
        public void TestID()
        {
            using (var vw = new VowpalWabbit("--id abc"))
            {
                Assert.AreEqual("abc", vw.ID);

                vw.SaveModel("model");

                vw.ID = "def";
                vw.SaveModel("model.1");
            }

            using (var vw = new VowpalWabbit("-i model"))
            {
                Assert.AreEqual("abc", vw.ID);
            }

            using (var vw = new VowpalWabbit("-i model.1"))
            {
                Assert.AreEqual("def", vw.ID);
            }

            using (var vwm = new VowpalWabbitModel("-i model.1"))
            {
                Assert.AreEqual("def", vwm.ID);
                using (var vw = new VowpalWabbit(new VowpalWabbitSettings { Model = vwm }))
                {
                    Assert.AreEqual("def", vw.ID);
                    Assert.AreEqual(vwm.ID, vw.ID);
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Model Loading")]
        public void TestEmptyID()
        {
            using (var vw = new VowpalWabbit("-l 1"))
            {
                Assert.AreEqual(string.Empty, vw.ID);

                vw.SaveModel("model");
            }

            using (var vw = new VowpalWabbit("-f model"))
            {
                Assert.AreEqual(string.Empty, vw.ID);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Model Loading")]
        public void TestReload()
        {
            using (var vw = new VowpalWabbit(""))
            {
                vw.SaveModel("model");
                vw.Reload();
            }

            using (var vw = new VowpalWabbit(""))
            {
                vw.ID = "def";
                vw.SaveModel("model.1");

                vw.Reload();

                Assert.AreEqual("def", vw.ID);
            }
        }

        private void InternalTestModel(string modelFile, bool shouldPass)
        {
            bool passed = false;
            try
            {
                using (var vw = new VowpalWabbitModel(string.Format("--quiet -t -i {0}", modelFile)))
                {
                    // should only reach this point if model is valid
                    passed = true;
                }
            }
            catch (VowpalWabbitException ex)
            {
                Assert.IsTrue(ex.Message.Contains("corrupted"));
            }

            if (shouldPass)
            {
                Assert.IsTrue(passed);
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
                    using (var vw = new VowpalWabbitModel(new VowpalWabbitSettings("--quiet -t") { ModelStream = modelStream }))
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
