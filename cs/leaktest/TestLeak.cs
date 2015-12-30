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
#if DEBUG
        /// <summary>
        /// Tests if the leak detector actually works.
        /// </summary>
        /// <remarks>Only possible in debug as VLD is not linked against in release.</remarks>
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

        [TestMethod]
        public void TestNoLeak()
        {
            Run("cs_unittest.TestLeakClass", "NoLeak");
        }
#endif
    }
}
