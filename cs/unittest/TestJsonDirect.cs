using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using VW;
using VW.Interfaces;
using VW.Labels;

namespace cs_unittest
{
    [TestClass]

    public class TestJsonDirectClass
    {
        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDirect()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonContext>(new VowpalWabbitSettings(featureDiscovery:VowpalWabbitFeatureDiscovery.Json)))
            {
                //vw.Validate("| Clicks:5 |a Bar:1 25_old |b Marker", new JsonContext() {
                //    Ns1 = new Namespace1
                //    {
                //        Foo = 1,
                //        Age = "25 old",
                //        DontConsider = "XXX"
                //    },
                //    Ns2 = new Namespace2
                //    {
                //        FeatureA = true
                //    },
                //    Clicks = 5
                //});

                vw.Validate("| Clicks:5 |a Bar:1", new JsonContext() {
                    Ns1 = new Namespace1
                    {
                        Foo = 1,
                        DontConsider = "XXX"
                    },
                    Clicks = 5,
                    IgnoreMe = "true"
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDirectWithLabel()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonContext>(new VowpalWabbitSettings(featureDiscovery: VowpalWabbitFeatureDiscovery.Json)))
            {
                vw.Validate("13 | Clicks:5 MoreClicks:3", new JsonContext()
                {
                    Label = new SimpleLabel { Label = 13 },
                    Clicks = 5,
                    MoreClicks = 3,
                    IgnoreMe2 = "YYY"
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonOptIn()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonContextOptIn>(new VowpalWabbitSettings(featureDiscovery: VowpalWabbitFeatureDiscovery.Json)))
            {
                vw.Validate("| Clicked |Ns2 Marker", new JsonContextOptIn()
                {
                    Clicked = true,
                    IgnoredNamespace = new Namespace1 {  Foo = 3 },
                    Ns2 = new Namespace2
                    {
                        FeatureA = true
                    }
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestLabelJsonSerialization()
        {
            Assert.AreEqual(
                "{\"_label\":{\"Label\":25.0},\"Clicks\":0,\"MoreClicks\":3}",
                JsonConvert.SerializeObject(new JsonContext() {
                    Label = new SimpleLabel { Label = 25 },
                    MoreClicks = 3,
                    IgnoreMe = "XXX" }));
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonArray()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonContextArray>(new VowpalWabbitSettings(featureDiscovery: VowpalWabbitFeatureDiscovery.Json)))
            {
                vw.Validate("1:2:.5 | :.1 :.2 :.3", new JsonContextArray()
                {
                    Label = new ContextualBanditLabel { Action =1 , Cost = 2, Probability = .5f },
                    Data = new[] { .1f, .2f, .3f }
                });
            }
        }
    }

    [JsonObject(MemberSerialization = MemberSerialization.OptOut)]
    public class JsonContextArray
    {
        [JsonProperty("_label")]
        public ILabel Label { get; set; }

        public float[] Data { get; set; }
    }

    [JsonObject(MemberSerialization = MemberSerialization.OptIn)]
    public class JsonContextOptIn
    {
        public int IgnoreMe { get; set; }

        [JsonProperty]
        public bool Clicked { get; set; }

        public Namespace1 IgnoredNamespace { get; set; }

        [JsonProperty]
        public Namespace2 Ns2 { get; set; }
    }

    public class JsonContext
    {
        [JsonProperty(PropertyName = "_label")]
        public SimpleLabel Label { get; set; }

        [JsonProperty(PropertyName = "a", NullValueHandling = NullValueHandling.Ignore)]
        public Namespace1 Ns1 { get; set; }

        [JsonProperty(PropertyName = "b", NullValueHandling = NullValueHandling.Ignore)]
        public Namespace2 Ns2 { get; set; }

        [JsonProperty]
        public int Clicks { get; set; }

        public int MoreClicks { get; set; }

        [JsonIgnore]
        public object IgnoreMe { get; set; }

        [JsonProperty(PropertyName = "_aux", NullValueHandling = NullValueHandling.Ignore)]
        public object IgnoreMe2 { get; set; }
    }

    public class Namespace1
    {
        [JsonProperty(PropertyName = "Bar", NullValueHandling = NullValueHandling.Ignore)]
        public int Foo { get; set; }

        [JsonProperty]
        public string Age { get; set; }

        [JsonIgnore]
        public string DontConsider { get; set; }
    }

    public class Namespace2
    {
        [JsonProperty("Marker")]
        public bool FeatureA { get; set; }
    }
}
