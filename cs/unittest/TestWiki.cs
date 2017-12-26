using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;

namespace cs_unittest
{
    [TestClass]
    public class TestWikiClass
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestWiki()
        {
            using (var vw = new VW.VowpalWabbit("-f rcv1.model"))
            {
                // 1 |f 13:3.9656971e-02 24:3.4781646e-02 69:4.6296168e-02
                using (var exampleBuilder = new VW.VowpalWabbitExampleBuilder(vw))
                {
                    // important to dispose the namespace builder at the end, as data is only added to the example
                    // if there is any feature added to the namespace
                    using (var ns = exampleBuilder.AddNamespace('f'))
                    {
                        var namespaceHash = vw.HashSpace("f");

                        var featureHash = vw.HashFeature("13", namespaceHash);
                        ns.AddFeature(featureHash, 8.5609287e-02f);

                        featureHash = vw.HashFeature("24", namespaceHash);
                        ns.AddFeature(featureHash, 3.4781646e-02f);

                        featureHash = vw.HashFeature("69", namespaceHash);
                        ns.AddFeature(featureHash, 4.6296168e-02f);
                    }

                    exampleBuilder.ApplyLabel(new SimpleLabel() { Label = 1 });

                    // hand over of memory management
                    using (var example = exampleBuilder.CreateExample())
                    {
                        VowpalWabbitExampleValidator.Validate("1 |f 13:8.5609287e-02 24:3.4781646e-02 69:4.6296168e-02", example, VowpalWabbitLabelComparator.Simple);

                        vw.Learn(example);
                    }
                }
            }
        }
    }
}
