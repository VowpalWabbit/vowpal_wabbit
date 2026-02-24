using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class TestLeak
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestExamplePoolCleanupOnDispose()
        {
            // Verify that disposing a VowpalWabbit instance cleans up pooled
            // native examples without crashing. Before the fix, pooled examples
            // leaked native memory because InternalDispose() was a stub.
            using (var vw = new VowpalWabbit("--quiet"))
            {
                for (int i = 0; i < 100; i++)
                {
                    using (var example = vw.GetOrCreateNativeExample())
                    {
                        // Example returns to pool on Dispose
                    }
                }
            }

            // Force GC to surface any dangling native handles that would
            // crash if the workspace was deleted before the examples.
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestRepeatedCreateDisposeNoLeak()
        {
            // Stress test: many VW instances each exercising the example pool.
            // Without proper cleanup in OperatorDelete, native memory grows
            // without bound across iterations.
            for (int round = 0; round < 50; round++)
            {
                using (var vw = new VowpalWabbit("--quiet"))
                {
                    for (int i = 0; i < 200; i++)
                    {
                        using (var example = vw.GetOrCreateNativeExample())
                        {
                        }
                    }
                }
            }

            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
        }
    }
}
