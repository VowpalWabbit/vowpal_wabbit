using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer;

namespace cs_unittest
{
    [TestClass]
    public class TestConfigInspector
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestConfigParsing()
        {
            var str = @"
f1
ns1.f2(FeatureGroup = 'a')
ns1.f3(Namespace = ""a"",Enumerize=true, FeatureGroup = 'g'  )
ns1.f4(Enumerize=true , FeatureGroup ='f', Order=3,AddAnchor=true)
ns1.ns2.f5
            ";

            var schema = ConfigInspector.CreateSchema(typeof(ConfigSample), str, msg => Assert.Fail(msg));

            using (var vw = new VowpalWabbitExampleValidator<ConfigSample>(new VowpalWabbitSettings { Schema = schema }))
            {
                vw.Validate("| f1:1 f5:5 |a abc |ga f33 |f f44 ",
                    new ConfigSample
                    {
                        f1 = 1,
                        ns1 = new ConfigSampleNamespace
                        {
                            f2 = "abc",
                            f3 = 3,
                            f4 = 4,
                            ns2 = new ConfigSampleNamespaceSub
                            {
                                f5 = 5
                            }
                        }
                    });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestConfigADFParsing()
        {
            var schemaShared = ConfigInspector.CreateSchema(typeof(ConfigShared), "f1", msg => Assert.Fail(msg));
            var schemaADF = ConfigInspector.CreateSchema(typeof(ConfigADF), "f2(Enumerize=true)", msg => Assert.Fail(msg));

            using (var vw = new VowpalWabbit<ConfigShared, ConfigADF>(new VowpalWabbitSettings("--cb_adf") { Schema = schemaShared, ActionDependentSchema = schemaADF }))
            using (var vwNative = new VowpalWabbit("--cb_adf"))
            {
                vw.Learn(
                    new ConfigShared { f1 = 2, ignore_me = 3 },
                    new[]
                    {
                        new ConfigADF { f2 = 3 },
                        new ConfigADF { f2 = 4 },
                    }, 0, new ContextualBanditLabel { Action = 0, Cost = 1, Probability = .5f });

                vwNative.Learn(
                    new[]
                    {
                        "shared | f1:2",
                        "0:1:.5 | f23",
                        " | f24"
                    });

                vw.Native.SaveModel("config-actual.model");
                vwNative.SaveModel("config-expected.model");
            }

            var actual = File.ReadAllBytes("config-actual.model");
            var expected = File.ReadAllBytes("config-expected.model");

            CollectionAssert.AreEqual(expected, actual);
        }
    }

    public class ConfigSample
    {
        public int f1 { get; set; }

        public ConfigSampleNamespace ns1 { get; set; }
    }

    public class ConfigSampleNamespace
    {
        public string f2 { get; set; }

        public int f3 { get; set; }

        public int f4 { get; set; }

        public ConfigSampleNamespaceSub ns2 { get; set; }
    }

    public class ConfigSampleNamespaceSub
    {
        public int f5 { get; set; }
    }

    public class ConfigShared
    {
        public int f1 { get; set; }

        public int ignore_me { get; set; }
    }

    public class ConfigADF
    {
        public int f2 { get; set; }
    }
}
