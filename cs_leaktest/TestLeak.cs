using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_leaktest
{
    [TestClass]
    public class TestLeakClass : TestWrappedBase
    {
        [TestMethod]
        public void TestLeak()
        {
            try
            {                
                Run("cs_unittest.TestLeakClass", "Leak");
            }
            catch (AssertFailedException ex)
            {
                Assert.IsTrue(ex.Message.Contains("Total 492 bytes")); // 123 *4
            }
        }
    }
}
