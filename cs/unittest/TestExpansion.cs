using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestExpansionClass
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestExpansion()
        {
            using (var vw = new VowpalWabbitExampleValidator<ExpansionContext>(string.Empty))
            {
                vw.Validate("| 3:.1 4:.2 5:.3", new ExpansionContext() { Features = new[] { .1f, .2f, .3f }, Offset = 3 });
            }
        }
    }

    public class ExpansionContext
    {
        public float[] Features { get; set; }

        public int Offset { get; set; }

        [Feature]
        public IEnumerable<float> ExpandedFeatures
        {
            get
            {
                return Enumerable.Repeat(0f, this.Offset)
                    .Concat(Features);
            }
        }
    }
}
