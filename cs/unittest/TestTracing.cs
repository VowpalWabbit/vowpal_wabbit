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
    public class TestTracing
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestTraceListener()
        {
            var trace = "";
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings
            {
                TraceListener = msg => trace += msg,
                Verbose = true
            }))
            {
                vw.Learn("1 |a x:2");
                vw.Learn("2 |a x:3");
            }

            const int expected = 24;
            var actualLines = trace.Split('\n');
            Assert.AreEqual(expected, actualLines.Length, $"Expected {expected} lines. Found {actualLines.Length}. '{trace}'");
            // Last entry is an empty line. So go back to second last.
            Assert.AreEqual("total feature number = 4", actualLines[actualLines.Length - 2]);
        }
    }
}
