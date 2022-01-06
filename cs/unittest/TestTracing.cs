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
            var messages = new List<string>();

            using (var vw = new VowpalWabbit(new VowpalWabbitSettings
            {
                TraceListener = msg => messages.Add(msg),
                Verbose = true
            }))
            {
                vw.Learn("1 |a x:2");
                vw.Learn("2 |a x:3");
            }

            var trace = string.Join("\n", messages);
            var expected = 26;
            Assert.AreEqual(expected, messages.Count, $"Expected {expected} lines. Found {messages.Count}. '{trace}'");
            Assert.AreEqual("total feature number = 4", messages[25]);
        }
    }
}
