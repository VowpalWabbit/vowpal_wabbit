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
            string trace = "";
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings
            {
                TraceListener = msg => trace += msg,
                Verbose = true
            }))
            {
                vw.Learn("1 |a x:2");
                vw.Learn("2 |a x:3");
            }

            var expected = 25;
            var actualLines = trace.Split('\n');
            Assert.AreEqual(expected, actualLines.Count(), $"Expected {expected} lines. Found {actualLines.Count()}. '{trace}'");
            Assert.AreEqual("total feature number = 4", actualLines.Last());
        }
    }
}
