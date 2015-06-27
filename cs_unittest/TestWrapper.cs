using cs_test;
using VW;
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
    public class TestWrapper
    {
        [TestMethod]
        public void VwCleanupTest()
        {
            new VowpalWabbit<Test1>("-k -l 20 --initial_t 128000 --power_t 1 -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off")
                .Dispose();
        }

        [TestMethod]
        public void VwCleanupTestError()
        {
            try
            {
                if (Directory.Exists("models"))
                    Directory.Delete("models", true);
                var vw = new VowpalWabbit<Test1>("-k -l 20 --initial_t 128000 --power_t 1 -f models/0001.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off");
                vw.Dispose();

                Assert.Fail("Excepted exception not thrown");
            }
            catch (VowpalWabbitException e)
            {
                Assert.IsFalse(string.IsNullOrEmpty(e.Filename));
                Assert.AreNotEqual(0, e.LineNumber);
                Assert.IsTrue(e.Message.Contains("No such file or directory"), e.Message);
            }
        }

        [TestMethod]
        public void VwModelRefCounting()
        {
            var model = new VowpalWabbitModel("");

            //var i1 = new VowpalWabbit(model);
            //var i2 = new VowpalWabbit(model);

            //i1.Dispose();
            model.Dispose();
            //i1.Dispose();
        }
    }
}
