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
    public partial class TestAll
    {
        private VisualLeakDetector visualLeakDetector;

        [TestInitialize]
        public void TestInit()
        {
            visualLeakDetector = new VisualLeakDetector();

            Directory.CreateDirectory("models");
        }

        [TestCleanup]
        public void TestCleanup()
        {
            if (visualLeakDetector != null)
            {
                visualLeakDetector.ReportLeaks();
                Assert.AreEqual(0, visualLeakDetector.Messages.Count,
                    string.Join("\n", visualLeakDetector.Messages.Select(t => t.Item2)));

                visualLeakDetector.Dispose();

                visualLeakDetector = null;
            }
        }
    }
}
