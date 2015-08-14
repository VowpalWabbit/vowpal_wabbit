using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
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
        public void TestLoadModel()
        {
            InternalTestModel(@"model-sets/7.10.2_corrupted.model", false);
            InternalTestModel(@"model-sets/8.0.0_ok.model", true);
        }

        private void InternalTestModel(string modelFile, bool shouldPass)
        {
            try
            {
                var vw = new VowpalWabbitModel(string.Format("--quiet -t -i {0}", modelFile));

                // should only reach this point if model is valid
                Assert.IsTrue(shouldPass);
            }
            catch (VowpalWabbitException ex)
            {
                Assert.IsTrue(ex.Message.Contains("corrupted"));
            }
        }
    }

    
}
