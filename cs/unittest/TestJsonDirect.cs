using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using System.Collections.Generic;
using VW;
using VW.Labels;
using VW.Serializer;

namespace cs_unittest
{
    [TestClass]

    public class TestJsonDirectClass
    {
        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDirect()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonContext>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate("| Clicks:5 |a Bar:1 Age25_old |b Marker", new JsonContext()
                {
                    Ns1 = new Namespace1
                    {
                        Foo = 1,
                        Age = "25 old",
                        DontConsider = "XXX"
                    },
                    Ns2 = new Namespace2
                    {
                        FeatureA = true
                    },
                    Clicks = 5
                });

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
            using (var vw = new VowpalWabbitExampleValidator<JsonContext>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
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
            using (var vw = new VowpalWabbitExampleValidator<JsonContextOptIn>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
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
            using (var vw = new VowpalWabbitExampleValidator<JsonContextArray>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate("1:2:.5 | :.1 :.2 :.3", new JsonContextArray()
                {
                    Label = new ContextualBanditLabel { Action =1 , Cost = 2, Probability = .5f },
                    Data = new[] { .1f, .2f, .3f }
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonByte()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonContextByte>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate("| Feature:25", new JsonContextByte { Feature = 25 });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDirectText()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonText>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate("| a b c |a d e f", new JsonText
                {
                    Text = "a b c",
                    AuxInfo = "Foo",
                    A = new JsonText
                    {
                        Text = "d e f",
                        AuxInfo = "Bar"
                    }
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDirectMulti()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonShared>(new VowpalWabbitSettings("--cb_adf") { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate(new[]
                {
                    "shared | Ageteen",
                    " | Id:1",
                    " | Id:2"
                },
                new JsonShared
                {
                   Age = "teen",
                   Documents = new []
                   {
                       new JsonADF { Id = 1 },
                       new JsonADF { Id = 2 }
                   }
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDirectMultiList()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonSharedList>(new VowpalWabbitSettings("--cb_adf") { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate(new[]
                {
                    " | Id:1",
                    " | Id:2"
                },
                new JsonSharedList
                {
                    _multi = new List<JsonADF>
                   {
                       new JsonADF { Id = 1 },
                       new JsonADF { Id = 2 }
                   }
                });

                vw.Validate(new[]
                {
                    "shared | Ageteen",
                    " | Id:1"
                },
                new JsonSharedList
                {
                    Age = "teen",
                    _multi = new List<JsonADF>
                    {
                        new JsonADF { Id = 1 }
                    }
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDirectMultiEmpty()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonSharedEmpty>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate(new[]
                {
                    " | Id:1",
                    " | Id:2"
                },
                new JsonSharedEmpty
                {
                    Age = "ignored",
                    _multi = new[]
                    {
                        new JsonADF { Id = 1 },
                        new JsonADF { Id = 2 }
                    }
                });
            }
        }
    }

    public class JsonText
    {
        [JsonProperty("_text")]
        public string Text { get; set; }

        [JsonProperty("_auxInfo")]
        public string AuxInfo { get; set; }

        [JsonProperty("a")]
        public JsonText A { get; set; }
    }

    [JsonObject(MemberSerialization = MemberSerialization.OptOut)]
    public class JsonContextArray
    {
        [JsonIgnore]
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

    public class JsonContextByte
    {
        public byte Feature { get; set; }
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

    public class JsonShared
    {
        public string Age { get; set; }

        [JsonProperty("_multi")]
        public JsonADF[] Documents { get; set; }
    }

    public class JsonSharedList
    {
        public string Age { get; set; }

        public List<JsonADF> _multi { get; set; }
    }

    public class JsonSharedEmpty
    {
        [JsonProperty("_ignoreMe")]
        public string Age { get; set; }

        public IEnumerable<JsonADF> _multi { get; set; }
    }

    public class JsonADF
    {
        public int Id { get; set; }
    }
}
