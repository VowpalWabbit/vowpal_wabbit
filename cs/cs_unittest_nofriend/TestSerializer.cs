using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW.Serializer.Attributes;
using VW;

namespace cs_unittest_nofriend
{
    class PrivateClass
    {
        [Feature]
        public int A { get; set; }
    }

    [TestClass]
    public class TestSerializer
    {
        [TestMethod]
        [ExpectedException(typeof(ArgumentException))]
        public void TestPrivateClassException()
        {
            using (var vw = new VowpalWabbit<PrivateClass>(""))
            {
            }
        }
    }
}
