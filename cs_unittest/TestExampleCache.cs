using Microsoft.Research.MachineLearning;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public class TestExampleCache : TestBase
    {
        [TestMethod]
        [ExpectedException(typeof(NotSupportedException))]
        public void TestExampleCacheForLearning()
        {
            using (var vw = new VowpalWabbit<CachedData>(string.Empty))
            {
                using (var example = vw.ReadExample(new CachedData()))
                {
                    example.Learn<VowpalWabbitPredictionNone>();
                }
            }
        }
    }
     
    [Cacheable]
    public class CachedData
    {
        [Feature]
        public float Feature { get; set; }
    }
}
