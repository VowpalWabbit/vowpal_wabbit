// --------------------------------------------------------------------------------------------------------------------
// <copyright file="TestJsonDirectClass.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.Linq;
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

                vw.Validate("| Clicks:5 |a Bar:1", new JsonContext()
                {
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
                    IgnoredNamespace = new Namespace1 { Foo = 3 },
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
                JsonConvert.SerializeObject(new JsonContext()
                {
                    Label = new SimpleLabel { Label = 25 },
                    MoreClicks = 3,
                    IgnoreMe = "XXX"
                }));
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonArray()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonContextArray>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate("1:2:.5 |Data :.1 :.2 :.3", new JsonContextArray()
                {
                    Label = new ContextualBanditLabel { Action = 1, Cost = 2, Probability = .5f },
                    Data = new[] { .1f, .2f, .3f }
                });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        [TestCategory("Marshal")]
        public void TestJsonDictionaryStringFloat()
        {
            using (var vw = new VowpalWabbitExampleValidator<POCODict>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate("|Features Feature1:2.1 Feature2:3.2", new POCODict { Features = new Dictionary<string, float> { { "Feature1", 2.1f }, { "Feature2", 3.2f } } });
            }

            // test serialzier caching too
            using (var vw = new VowpalWabbitExampleValidator<POCODict>(new VowpalWabbitSettings { TypeInspector = TypeInspector.All }))
            {
                vw.Validate("| Abc:2.1 def:3.2", new POCODict { Features = new Dictionary<string, float> { { "Abc", 2.1f }, { "def", 3.2f } } });
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
                    Documents = new[]
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

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonConvertibleMulti()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonRawAdfString>(new VowpalWabbitSettings("--cb_adf") { TypeInspector = JsonTypeInspector.Default }))
            {
                vw.Validate(new[] {
                    "shared | Bar:5",
                    " | Foo:1 |Value test:1.2",
                    " | Foo:2 |Value test:2.3",
                    " | Foo:3 |Value titleabc\"def",
                },
                new JsonRawAdfString
                {
                    Bar = 5,
                    _multi = new[]
                    {
                        new JsonRawString
                        {
                            Foo = 1,
                            Value = JsonConvert.SerializeObject(new { test = 1.2 })
                        },
                        new JsonRawString
                        {
                            Foo = 2,
                            Value = JsonConvert.SerializeObject(new { test = 2.3 })
                        },
                        new JsonRawString
                        {
                            Foo = 3,
                            Value = JsonConvert.SerializeObject(new { title = "abc\"def", _ignoreMe = 1 })
                        },
                    }
                });

                var adf = new JsonRawString
                {
                    Foo = 1,
                    Value = JsonConvert.SerializeObject(new { A = new { test = 1.2 }, B = new { bar = 2 } }),
                    Values = new[]
                                    {
                                        JsonConvert.SerializeObject(new { D = new { d = 1.2 } }),
                                        JsonConvert.SerializeObject(new { E = new { e = true } }),
                                    }.ToList()
                };

                var ctx = new JsonRawAdfString
                {
                    Bar = 5,
                    _multi = new[] { adf }
                };

                vw.Validate(new[] {
                    "shared | Bar:5",
                    " | Foo:1 |A test:1.2 |B bar:2 |D d:1.2 |E e"
                }, ctx);
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonConvertible()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonRawString>(new VowpalWabbitSettings("") { TypeInspector = JsonTypeInspector.Default }))
            {
                var adf = new JsonRawString
                {
                    Foo = 1,
                    Value = JsonConvert.SerializeObject(new { A = new { test = 1.2 }, B = new { bar = 2 } }),
                    Values = new[]
                    {
                        JsonConvert.SerializeObject(new { D = new { d = 1.2 } }),
                        JsonConvert.SerializeObject(new { E = new { e = true } }),
                        JsonConvert.SerializeObject(new { F = new { title = "abc\"def" } }),
                    }.ToList()
                };

                vw.Validate(" | Foo:1 |A test:1.2 |B bar:2 |D d:1.2 |E e |F titleabc\"def", adf);
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonDictArray()
        {
            using (var vw = new VowpalWabbitExampleValidator<JsonDictArray>(new VowpalWabbitSettings(string.Empty) { TypeInspector = JsonTypeInspector.Default }))
            {
                var ex = new JsonDictArray
                {
                    Features = new Dictionary<string, float[]>
                        {
                            { "A", new float[] { 1, 2, 3.1f} },
                            { "B", new float[] { 2, 3, 4.1f} }
                        }
                };

                vw.Validate(" |A :1 :2 :3.1 |B :2 :3 :4.1", ex);
            }
        }
    }

    public class JsonRawString
    {
        public int Foo { get; set; }

        [JsonConverter(typeof(JsonRawStringConverter))]
        public string Value { get; set; }

        [JsonConverter(typeof(JsonRawStringListConverter))]
        public List<string> Values { get; set; }
    }

    public class JsonRawAdfString
    {
        public int Bar { get; set; }

        public JsonRawString[] _multi { get; set; }
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

        [JsonProperty]
        public string EscapeCharacterString { get; set; }

        [JsonProperty("_text")]
        public string EscapeCharactersText { get; set; }
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

    public class JsonDictArray
    {
        public Dictionary<string, float[]> Features { get; set; }
    }

}
