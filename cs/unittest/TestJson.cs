using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer;
using VW.Serializer.Attributes;
using VW.Serializer.Intermediate;

namespace cs_unittest
{
    [TestClass]
    public class TestJsonClass
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJson()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1}}");
                validator.Validate("|a foo:2.3", "{\"a\":{\"foo\":2.3}}");
                validator.Validate("|a foo:2.3 bar", "{\"a\":{\"foo\":2.3, \"bar\":true}}");
                validator.Validate("|a foo:1 |bcd Age25_old", "{\"a\":{\"foo\":1},\"bcd\":{\"Age\":\"25 old\"}}");
                validator.Validate("|a x{abc}", "{\"a\":{\"x\":\"{abc}\"}}");
                validator.Validate("|a x{abc}", "{\"a\":{\"x\":\"{abc}\",\"y\":null}}");
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonAux()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":5}");
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":\"\"}");
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":{\"abc\":{\"def\":3}}}");
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":[1,2,[3,4],2]}");
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":[1,2,[3,[1],{\"ab,\":3}],2]}");
                validator.Validate("|a foo:1 | b:1", "{\"a\":{\"foo\":1},\"_aux\":{\"a\":\"{\\\"} \"}, \"b\":1}");
            }
        }

        private void AssertThrow(Action action, Type expectedException = null)
        {
            if (expectedException == null)
                expectedException = typeof(VowpalWabbitException);

            try
            {
                action();
                Assert.Fail("Expected exception " + expectedException);
            }
            catch (Exception e)
            {
                Assert.IsInstanceOfType(e, expectedException);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonInvalid()
        {
            using (var vw = new VowpalWabbit("--json"))
            {
                AssertThrow(() => vw.ParseJson("{\"_label\":true,\"a\":{\"foo\":1}}"));
                AssertThrow(() => vw.ParseJson("{\"_labelfoo\":1,\"a\":{\"foo\":1}}"));
                AssertThrow(() => vw.ParseJson("{\"_label_foo\":1,\"a\":{\"foo\":1}}"));
                AssertThrow(() => vw.ParseJson("{\"_label\":{\"label\":{\"a\":1}},\"a\":{\"foo\":1}}"));
            }

            using (var vw = new VowpalWabbit("--cb_adf --json"))
            {
                AssertThrow(() => vw.ParseJson("{\"_label_Action\":1,\"_label_Cost\":-2,\"_label_Probability\":0.3,\"_multi\":[{\"foo\":1}],\"foo\":2,\"_labelIndex\":1}"));
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonArray()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("|a :1 :2.3 :4", "{\"a\":[1,2.3,4]}");
                validator.Validate("|b :1 :2.3 :4", "{\"a\":{\"b\":[1,2.3,4]}}");
            }

            using (var vw = new VowpalWabbit("--json"))
            {
                AssertThrow(() => vw.ParseJson("{\"a\":{\"b\":[1,[1,2],4]}}"));
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonSimpleLabel()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("1 |a foo:1", "{\"_label\":{\"Label\":1},\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
                validator.Validate("1.2 |a foo:1", "{\"_label\":1.2,\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
                validator.Validate("1.2 |a foo:1", "{\"_label\":1.2,\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonVWLabel()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("1 |a foo:1", "{\"_label\":1,\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
                validator.Validate("1 |a foo:1", "{\"_label\":\"1\",\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonSimpleLabelOverride()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("2 |a foo:1", "{\"_label\":{\"Label\":1},\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple,
                    new SimpleLabel { Label = 2 },
                    enableNativeJsonValidation: false /* vw.Parse(json) doesn't support label overwrite */);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonContextualBanditLabel()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator("--cb 2 --cb_type dr"))
            {
                validator.Validate("1:2:.5 |a foo:1",
                    "{\"_label\":\"1:2:.5\",\"a\":{\"foo\":1}}",
                    VowpalWabbitLabelComparator.ContextualBandit);

                validator.Validate("1:-2:.3 |a foo:1",
                    "{\"_label\":{\"Action\":1,\"Cost\":-2,\"Probability\":0.3},\"a\":{\"foo\":1}}",
                    VowpalWabbitLabelComparator.ContextualBandit);

                validator.Validate("1:-2:.3 |a foo:1",
                    "{\"_label_Action\":1,\"_label_Cost\":-2,\"_label_Probability\":0.3,\"a\":{\"foo\":1}}",
                    VowpalWabbitLabelComparator.ContextualBandit);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonADF()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator("--cb_adf"))
            {
                validator.Validate(new[]
                    {
                        "shared | foo:2",
                        "1:-2:.3 | foo:1"
                    },
                    "{\"_label_Action\":1,\"_label_Cost\":-2,\"_label_Probability\":0.3,\"_multi\":[{\"foo\":1}],\"foo\":2,\"_labelIndex\":0}",
                    VowpalWabbitLabelComparator.ContextualBandit,
                    index: 1);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonToVWString()
        {
            var jsonContext = new JsonContext()
            {
                Label = new SimpleLabel
                {
                    Label = 25
                },
                Ns1 = new Namespace1
                {
                    Foo = 1,
                    Age = "25",
                    DontConsider = "XXX",
                    EscapeCharacterString = "a: a | a\ta",
                    EscapeCharactersText = "b: b | b\tb"
                },
                Ns2 = new Namespace2
                {
                    FeatureA = true
                },
                Clicks = 5
            };

            var jsonContextString = JsonConvert.SerializeObject(jsonContext);
            using (var validator = new VowpalWabbitExampleJsonValidator(new VowpalWabbitSettings
            {
                Arguments = "--json",
                EnableStringExampleGeneration = true,
                EnableStringFloatCompact = true,
                EnableThreadSafeExamplePooling = true
            }))
            {
                validator.Validate("25  |  Clicks:5 MoreClicks:0  |a Bar:1 Age25 EscapeCharacterStringa__a___a_a b_ b _ b b  |b Marker",
                    jsonContextString,
                    VowpalWabbitLabelComparator.Simple);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonMultiline()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator("--cb 2 --cb_type dr"))
            {
                validator.Validate(new[] {
                     "shared | Age:25",
                     " | w1 w2 |a x:1",
                     " | w2 w3"
                    },
                    "{\"Age\":25,\"_multi\":[{\"_text\":\"w1 w2\", \"a\":{\"x\":1}}, {\"_text\":\"w2 w3\"}]}");

                validator.Validate(new[] {
                     "shared | Age:25",
                     " | w1 w2 |a x:1",
                     "2:-1:.3 | w2 w3"
                    },
                    "{\"Age\":25,\"_multi\":[{\"_text\":\"w1 w2\", \"a\":{\"x\":1}}, {\"_text\":\"w2 w3\",\"_label\":\"2:-1:.3\"}]}",
                    VowpalWabbitLabelComparator.ContextualBandit);
            }

            using (var validator = new VowpalWabbitExampleJsonValidator(
                new VowpalWabbitSettings
                {
                    Arguments = "--cb 2 --cb_type dr",
                    PropertyConfiguration = new PropertyConfiguration
                    {
                        MultiProperty = "adf",
                        TextProperty = "someText",
                        LabelProperty = "theLabel",
                        FeatureIgnorePrefix = "xxx"
                    }
                }))
            {
                validator.Validate(new[] {
                     "shared | Age:25",
                     " | w1 w2 |a x:1",
                     "2:-1:.3 | w2 w3"
                    },
                    "{\"Age\":25,\"adf\":[{\"someText\":\"w1 w2\", \"a\":{\"x\":1}, \"xxxxIgnoreMe\":2}, {\"someText\":\"w2 w3\",\"theLabel\":\"2:-1:.3\"}]}",
                    VowpalWabbitLabelComparator.ContextualBandit,
                    enableNativeJsonValidation: false /* remapping of special properties is not supported in native JSON */);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonText()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator(""))
            {
                validator.Validate("| a b c |a d e f", "{\"_text\":\"a  b c\",\"a\":{\"_text\":\"d e f\"}}");
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonNumADFs()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator(""))
            {
                Assert.AreEqual(2,
                    VowpalWabbitJsonSerializer.GetNumberOfActionDependentExamples(
                    "{\"_text\":\"a b c\",\"a\":{\"_text\":\"d e f\"},_multi:[{\"a\":1},{\"b\":2,\"c\":3}]}"));

                Assert.AreEqual(0,
                    VowpalWabbitJsonSerializer.GetNumberOfActionDependentExamples(
                    "{\"_text\":\"a b c\",\"a\":{\"_text\":\"d e f\"},_multi:[]}"));

                Assert.AreEqual(0,
                    VowpalWabbitJsonSerializer.GetNumberOfActionDependentExamples(
                    "{\"_text\":\"a b c\",\"a\":{\"_text\":\"d e f\"}}"));
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonLabel()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator(""))
            {
                validator.Validate("1 | a:2 ", "{\"a\":2,\"_label_Label\":1}");
                validator.Validate("1:2:3 | a:2 ", "{\"a\":2,\"_label_Label\":1,\"_label_Initial\":2,\"_label_weight\":3}");
            }

            using (var validator = new VowpalWabbitExampleJsonValidator(new VowpalWabbitSettings
            {
                Arguments = "--cb_adf",
                PropertyConfiguration = new PropertyConfiguration
                {
                    MultiProperty = "adf",
                    TextProperty = "someText",
                    FeatureIgnorePrefix = "xxx"
                }
            }))
            {
                validator.Validate(new[] {
                     "shared | Age:25",
                     " | w1 w2 |a x:1",
                     "0:-1:.3 | w2 w3"
                    },
                    "{\"Age\":25,\"adf\":[{\"someText\":\"w1 w2\", \"a\":{\"x\":1}, \"xxxxIgnoreMe\":2}, {\"someText\":\"w2 w3\"}], \"_labelIndex\":1, \"_label_Cost\":-1, \"_label_Probability\":0.3}",
                    VowpalWabbitLabelComparator.ContextualBandit,
                    enableNativeJsonValidation: false);

                // all lower case (ASA issue)
                validator.Validate(new[] {
                     " | w1 w2 |a x:1",
                     "0:-1:.3 | w2 w3"
                    },
                     "{\"adf\":[{\"someText\":\"w1 w2\", \"a\":{\"x\":1}, \"xxxxIgnoreMe\":2}, {\"someText\":\"w2 w3\"}], \"_labelindex\":1, \"_label_cost\":-1, \"_label_probability\":0.3}",
                     VowpalWabbitLabelComparator.ContextualBandit,
                     enableNativeJsonValidation: false);

                validator.Validate(new[] {
                     "shared | Age:25",
                     " | w1 w2 |a x:1",
                     " | w2 w3"
                    },
                     "{\"Age\":25,\"adf\":[{\"someText\":\"w1 w2\", \"a\":{\"x\":1}, \"xxxxIgnoreMe\":2}, {\"someText\":\"w2 w3\"}], \"_labelindex\":null}",
                     VowpalWabbitLabelComparator.ContextualBandit,
                     enableNativeJsonValidation: false);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonLabelExtraction()
        {
            using (var vw = new VowpalWabbit("--cb_adf --rank_all"))
            {
                using (var jsonSerializer = new VowpalWabbitJsonSerializer(vw))
                {
                    string eventId = null;
                    jsonSerializer.RegisterExtension((state, property) =>
                    {
                        Assert.AreEqual(property, "_eventid");
                        Assert.IsTrue(state.Reader.Read());

                        eventId = (string)state.Reader.Value;
                        return true;
                    });

                    jsonSerializer.Parse("{\"_eventid\":\"abc123\",\"a\":1,\"_label_cost\":-1,\"_label_probability\":0.3}");

                    Assert.AreEqual("abc123", eventId);

                    using (var examples = jsonSerializer.CreateExamples())
                    {
                        var single = examples as VowpalWabbitSingleLineExampleCollection;
                        Assert.IsNotNull(single);

                        var label = single.Example.Label as ContextualBanditLabel;
                        Assert.IsNotNull(label);

                        Assert.AreEqual(-1, label.Cost);
                        Assert.AreEqual(0.3, label.Probability, 0.0001);
                    }
                }

                using (var jsonSerializer = new VowpalWabbitJsonSerializer(vw))
                {
                    jsonSerializer.Parse("{\"_multi\":[{\"_text\":\"w1 w2\", \"a\":{\"x\":1}}, {\"_text\":\"w2 w3\"}], \"_labelindex\":1, \"_label_cost\":-1, \"_label_probability\":0.3}");

                    using (var examples = jsonSerializer.CreateExamples())
                    {
                        var multi = examples as VowpalWabbitMultiLineExampleCollection;
                        Assert.IsNotNull(multi);

                        Assert.AreEqual(2, multi.Examples.Length);
                        var label = multi.Examples[0].Label as ContextualBanditLabel;
                        Assert.AreEqual(0, label.Cost);
                        Assert.AreEqual(0, label.Probability);

                        label = multi.Examples[1].Label as ContextualBanditLabel;
                        Assert.IsNotNull(label);

                        Assert.AreEqual(-1, label.Cost);
                        Assert.AreEqual(0.3, label.Probability, 0.0001);
                    }
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestJsonRedirection()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator(new VowpalWabbitSettings("--cb_adf")))
            {
                validator.Validate(new[] {
                     "shared | Age:25",
                     " | w1 w2 |a x:1",
                     "0:-1:.3 | w2 w3"
                    },
                    "{\"_ignoreMe\":5,\"_sub\":{\"Age\":25,\"_multi\":[{\"_text\":\"w1 w2\", \"a\":{\"x\":1}}, {\"_text\":\"w2 w3\"}]}, \"_labelIndex\":1, \"_label_Cost\":-1, \"_label_Probability\":0.3}",
                    VowpalWabbitLabelComparator.ContextualBandit,
                    extension: (state, property) =>
                    {
                        if (!property.Equals("_sub"))
                            return false;

                        Assert.AreEqual(state.MultiIndex, -1);

                        state.Parse();

                        return true;
                    });

                validator.Validate(new[] {
                     "shared | Age:25",
                     " | w1 w2 |a x:1",
                     "0:-1:.3 | w2 w3"
                    },
                    "{\"Age\":25,\"_multi\":[{\"_text\":\"w1 w2\", \"a\":{\"x\":1}}, {\"_text\":\"w2 w3\", \"_tag\":\"2\"}], \"_labelIndex\":1, \"_label_Cost\":-1, \"_label_Probability\":0.3}",
                    VowpalWabbitLabelComparator.ContextualBandit,
                    extension: (state, property) =>
                    {
                        if (!property.Equals("_tag"))
                            return false;

                        var tag = state.Reader.ReadAsString();

                        Assert.AreEqual(1, state.MultiIndex);
                        Assert.AreEqual("2", tag);

                        return true;
                    });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestDecisionServiceJson()
        {
            using (var vw = new VowpalWabbit("--cb_adf"))
            {
                var json = @"{""EventId"":""abc"",""a"":[1,2,3],""Version"":""1"",""c"":{""u"":{""loc"":""New York""},""_multi"":[{""x"":[{""x"":{""cat"":""1""}},null,{""y"":{""cat"":""3""}}]},{""x"":{""cat"":""2""}}]},""p"":[0.8,0.1,0.1]}";
                var obj = JsonConvert.DeserializeObject(json);
                var bytes = Encoding.UTF8.GetBytes(json);
                VowpalWabbitDecisionServiceInteractionHeader header;
                List<VowpalWabbitExample> examples = null;

                try
                {
                    examples = vw.ParseDecisionServiceJson(bytes, 0, bytes.Length, copyJson: false, header: out header);

                    Assert.AreEqual("abc", header.EventId);
                    CollectionAssert.AreEqual(new[] { 1, 2, 3 }, header.Actions, "Actions mismatch");
                    CollectionAssert.AreEqual(new[] { .8f, .1f, .1f }, header.Probabilities, "Probabilities mismatch");
                    Assert.AreEqual(0, header.ProbabilityOfDrop);

                    using (var validator = new VowpalWabbitExampleJsonValidator(new VowpalWabbitSettings("--cb_adf")))
                    {
                        var expected = new[] {
                         "shared |u locNew_York",
                         " |x cat1 |y cat3",
                         " |x cat2"
                        };

                        validator.Validate(expected, examples);
                    }
                }
                finally
                {
                    if (examples != null)
                        foreach (var ex in examples)
                            if (ex != null)
                                ex.Dispose();
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestDecisionServiceJson2()
        {
            var json = "{\"Version\":\"2\",\"EventId\":\"73369b13ec98433096a1496d27da0bfd\",\"a\":[9,11,13,6,4,5,12,1,2,10,8,3,7],\"c\":{\"_synthetic\":false,\"User\":{\"_age\":0},\"Geo\":{\"country\":\"United States\",\"_countrycf\":\"8\",\"state\":\"New Jersey\",\"city\":\"Somerdale\",\"_citycf\":\"5\",\"dma\":\"504\"},\"MRefer\":{\"referer\":\"http://www.complex.com/\"},\"OUserAgent\":{\"_ua\":\"Mozilla/5.0 (iPhone; CPU iPhone OS 10_3_2 like Mac OS X) AppleWebKit/603.2.4 (KHTML, like Gecko) Version/10.0 Mobile/14F89 Safari/602.1\",\"_DeviceBrand\":\"Apple\",\"_DeviceFamily\":\"iPhone\",\"_DeviceIsSpider\":false,\"_DeviceModel\":\"iPhone\",\"_OSFamily\":\"iOS\",\"_OSMajor\":\"10\",\"_OSPatch\":\"2\",\"DeviceType\":\"Mobile\"},\"_multi\":[{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweater-conor-mcgregor-may-be-set\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweater-conor-mcgregor-may-be-set\"},\"j\":[{\"_title\":\"The Floyd Mayweather vs. Conor McGregor Fight Date Has Finally Been Announced\"},{\"RVisionTags\":{\"person\":0.999368966,\"man\":0.998108864,\"wearing\":0.9368642,\"hat\":0.928866565,\"indoor\":0.893332958,\"close\":0.201101109},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0148094837,\"racyScore\":0.0144806523},\"_expires\":\"2017-06-17T21:42:30.3186957Z\"},{\"Emotion0\":{\"anger\":1.81591489E-07,\"contempt\":9.946987E-06,\"disgust\":6.11135547E-05,\"fear\":4.565633E-12,\"happiness\":0.999928534,\"neutral\":2.28114558E-07,\"sadness\":3.07409E-09,\"surprise\":1.46155665E-08},\"_expires\":\"2017-06-17T21:42:28.8496462Z\"},{\"Tags\":{\"Floyd Mayweather Jr.\":0.982,\"Conor McGregor\":0.938,\"Complex\":0.334,\"Twitter Inc.\":0.997,\"Dan Mullane\":0.006,\"Mixed martial arts\":1,\"Net Controls\":0.281,\"Boxing\":1,\"Ontario\":1,\"Dana White\":0.972,\"Las Vegas Valley\":0.995,\"Nevada Athletic Commission\":0.024,\"Mayweather Promotions\":0.118,\"MGM Grand Garden Arena\":0.997,\"Fighting game\":0.641,\"Nevada\":1,\"Coming out\":0.076},\"_expires\":\"2017-06-17T21:42:29.8271823Z\"},{\"XSentiment\":2.93618323E-05,\"_expires\":\"2017-06-17T21:42:29.1777863Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/young-thug-beautiful-thugger-girls-violent-trailer-has-people-upset\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/young-thug-beautiful-thugger-girls-violent-trailer-has-people-upset\"},\"j\":[{\"_title\":\"Why Young Thug's Violent Trailer for 'Beautiful Thugger Girls' Has People Upset\"},{\"RVisionTags\":{\"person\":0.9621564,\"indoor\":0.93759197},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0590519942,\"racyScore\":0.0740057454},\"_expires\":\"2017-06-17T17:27:41.0750729Z\"},{\"Emotion0\":{\"anger\":0.000195411936,\"contempt\":0.0007970728,\"disgust\":7.29157255E-05,\"fear\":0.000106336483,\"happiness\":0.000127831052,\"neutral\":0.9209777,\"sadness\":0.0775285438,\"surprise\":0.000194183245},\"_expires\":\"2017-06-17T17:27:33.4982953Z\"},{\"Tags\":{\"Young Thug\":0.445,\"Beautiful\":0.005,\"Twitter Inc.\":1,\"Drake\":0.968,\"Instagram\":0.995,\"Breezy\":0.014,\"June 13\":0.008,\"Cover art\":0.003,\"Album\":0.99,\"Surface\":0.012,\"Prince Michael Jackson II\":1},\"_expires\":\"2017-06-17T17:27:33.4670277Z\"},{\"XSentiment\":2.46002031E-07,\"_expires\":\"2017-06-17T17:27:34.0139318Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/did-we-just-witness-peak-lebron\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/did-we-just-witness-peak-lebron\"},\"j\":[{\"_title\":\"Did We Just Witness Peak LeBron?\"},{\"RVisionTags\":{\"person\":0.9887383,\"indoor\":0.8961688},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0181511845,\"racyScore\":0.0380923077},\"_expires\":\"2017-06-17T14:46:55.4499648Z\"},{\"Emotion0\":{\"anger\":0.0007875018,\"contempt\":0.000600368367,\"disgust\":0.00288117747,\"fear\":0.00168624625,\"happiness\":4.59727671E-05,\"neutral\":0.377137423,\"sadness\":0.00264490047,\"surprise\":0.6142164},\"_expires\":\"2017-06-17T14:46:49.7072998Z\"},{\"Tags\":{\"LeBron James\":1,\"Complex\":0.08,\"Broadcasting of sports events\":0.12,\"Twitter Inc.\":0.999,\"USA Today\":0.994,\"Kyle Broflovski\":0.023,\"Superman\":0.694,\"Cleveland Cavaliers\":1,\"UNK NBA\":1,\"Bill Russell NBA Finals Most Valuable Player Award\":1,\"Golden State Warriors\":1,\"Michael Jordan\":1,\"Kobe Bryant\":1,\"Time\":0.865,\"Magic Johnson\":0.999},\"_expires\":\"2017-06-17T14:46:49.4416286Z\"},{\"XSentiment\":0.9999974,\"_expires\":\"2017-06-17T14:46:49.7541575Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-made-the-difference\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-made-the-difference\"},\"j\":[{\"_title\":\"Golden State (Probably) Would Have Blown Another Lead Without KD\"},{\"RVisionTags\":{\"person\":0.999950767,\"player\":0.984833837,\"sport\":0.9816471,\"athletic game\":0.9691899,\"basketball\":0.7260069,\"hand\":0.463383943,\"crowd\":0.3354485},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0127091147,\"racyScore\":0.01585682},\"_expires\":\"2017-06-17T14:44:47.7452593Z\"},{\"Emotion0\":{\"anger\":0.9830929,\"contempt\":6.38814454E-05,\"disgust\":0.0142428661,\"fear\":4.3443215E-05,\"happiness\":0.00127731345,\"neutral\":0.0003242454,\"sadness\":0.0006526729,\"surprise\":0.0003026811},\"_expires\":\"2017-06-17T14:44:47.1857099Z\"},{\"Tags\":{\"Golden State Warriors\":1,\"Lead guitar\":0.046,\"Kevin Durant\":1,\"Twitter Inc.\":0.985,\"Martinez\":0.015,\"Splash Brothers\":1,\"Cleveland Cavaliers\":1,\"Monday Night Football\":0.398,\"Richard Jefferson\":0.95,\"Kevin Love\":0.997,\"McDonald's All-American Game\":0.859,\"University of Texas at Austin College of Fine Arts\":0.999,\"Naismith College Player of the Year\":0.117,\"UNK NBA\":1,\"NBA Most Valuable Player Award\":1,\"Olympic Games\":0.858,\"NBA All-Star Game\":1,\"Champion\":0.12,\"Bill Russell NBA Finals Most Valuable Player Award\":1,\"Stephen Curry\":0.989,\"Draymond Green\":0.674,\"Seat Pleasant\":0.122},\"_expires\":\"2017-06-17T14:44:46.9264793Z\"},{\"XSentiment\":1,\"_expires\":\"2017-06-17T14:44:47.4639789Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\"},\"j\":[{\"_title\":\"Watch 2 Chainz Flex on Kendrick Lamar's \\\"DNA\\\" Beat in New Freestyle\"},{\"RVisionTags\":{\"person\":0.9840661,\"dressed\":0.304036647},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0285988934,\"racyScore\":0.0257010516},\"_expires\":\"2017-06-17T19:30:46.5524171Z\"},{\"_expires\":\"2017-06-17T19:30:46.2086571Z\"},{\"Tags\":{\"2 Chainz\":1,\"Kendrick Lamar\":1,\"DNA\":0.053,\"Hip hop production\":0.015,\"Freestyle rap\":0.934,\"Philadelphia\":0.193,\"Twitter Inc.\":1,\"Subscription business model\":0.011,\"Complex\":1,\"Los Angeles\":1,\"Georgia\":0.894,\"Trap\":0.579,\"Top Dawg Entertainment\":0.872,\"Virtual reality\":0.011,\"Travis Scott\":0.011,\"Collaboration\":0.004,\"Nicki Minaj\":0.999,\"Remy Ma\":0.99,\"Papoose\":0.892,\"Sampling\":0.017,\"Hip hop music\":1,\"Tha Carter V\":1,\"Everyday (ASAP Rocky song)\":0.003},\"_expires\":\"2017-06-17T19:30:46.1314041Z\"},{\"XSentiment\":1,\"_expires\":\"2017-06-17T19:30:46.7944713Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-viral-challenge-backfires-reacts\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-viral-challenge-backfires-reacts\"},\"j\":[{\"_title\":\"Floyd Mayweather Attempted to Start His Own Viral Challenge and It Hilariously Backfired\"},{\"RVisionTags\":{\"person\":0.999924064,\"man\":0.9552084,\"crowd\":0.01707872},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0114686647,\"racyScore\":0.0159121435},\"_expires\":\"2017-06-17T21:13:01.1925796Z\"},{\"Emotion0\":{\"anger\":0.00127403683,\"contempt\":0.0222781487,\"disgust\":0.0002176114,\"fear\":9.893078E-06,\"happiness\":0.07823167,\"neutral\":0.897131145,\"sadness\":0.00048493495,\"surprise\":0.0003725856},\"_expires\":\"2017-06-17T21:12:59.3669891Z\"},{\"Tags\":{\"Floyd Mayweather Jr.\":0.192,\"Philadelphia\":0.65,\"Twitter Inc.\":1,\"USA Today\":0.979,\"Sport\":0.85,\"Conor McGregor\":0.016,\"June 14\":0.009,\"Troy\":0.005,\"Honda Civic\":0.004,\"Bank account\":0.025,\"NASCAR on TNT\":0.099,\"Boxing\":1},\"_expires\":\"2017-06-17T21:12:59.3201526Z\"},{\"XSentiment\":0.999998,\"_expires\":\"2017-06-17T21:13:01.7647699Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-discusses-random-text-he-received-from-obama-after-winning-nba-finals\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-discusses-random-text-he-received-from-obama-after-winning-nba-finals\"},\"j\":[{\"_title\":\"Kevin Durant Discusses 'Random' Text He Received From Obama After Winning NBA Finals\"},{\"RVisionTags\":{\"person\":0.9939494,\"outdoor\":0.9162231,\"male\":0.242890328},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0208446719,\"racyScore\":0.0304868072},\"_expires\":\"2017-06-17T18:28:04.5675692Z\"},{\"Emotion0\":{\"anger\":2.35385942E-05,\"contempt\":3.294205E-06,\"disgust\":1.38025935E-05,\"fear\":7.8729E-06,\"happiness\":0.9997165,\"neutral\":0.0001607666,\"sadness\":1.25915176E-05,\"surprise\":6.161377E-05},\"Emotion1\":{\"anger\":0.00371822272,\"contempt\":0.000460597221,\"disgust\":0.000157746123,\"fear\":0.000275517668,\"happiness\":0.0403934456,\"neutral\":0.86978966,\"sadness\":0.08499769,\"surprise\":0.000207107121},\"_expires\":\"2017-06-17T18:28:03.3944386Z\"},{\"Tags\":{\"Kevin Durant\":1,\"Random House\":0.069,\"Barack Obama\":1,\"UNK NBA\":1,\"Twitter Inc.\":1,\"USA Today\":0.999,\"Sports journalism\":0.01,\"Cary\":0.025,\"The NBA Finals\":0.017,\"Monday Night Football\":0.943,\"Golden State Warriors\":1,\"Bill Simmons\":0.866,\"Podcast\":0.918,\"Oracle Arena\":0.028,\"Bill Russell NBA Finals Most Valuable Player Award\":1,\"June 13\":0.034,\"Cleveland Cavaliers\":1,\"LeBron James\":1,\"Kyrie Irving\":1,\"Allen Iverson\":0.999,\"Rihanna\":0.63,\"The League\":0.166,\"Stay\":0.858,\"Singing\":0.024,\"President of the United States\":1},\"_expires\":\"2017-06-17T18:28:03.8094333Z\"},{\"XSentiment\":0.9999663,\"_expires\":\"2017-06-17T18:28:03.9031587Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/pop-culture/2017/06/tj-miller-hbo-special-interview\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/pop-culture/2017/06/tj-miller-hbo-special-interview\"},\"j\":[{\"_title\":\"T.J. Miller's Done With 'Silicon Valley,' But His Career's Just Getting Started\"},{\"RVisionTags\":{\"person\":0.999560535,\"man\":0.9939658,\"suit\":0.950625,\"outdoor\":0.9169477,\"wearing\":0.7708228,\"jacket\":0.528340757,\"coat\":0.490623325,\"dark\":0.32463637,\"male\":0.212570518,\"microphone\":0.148984566,\"crowd\":0.0156043554},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0102083739,\"racyScore\":0.0126593616},\"_expires\":\"2017-06-17T18:59:23.2248419Z\"},{\"Emotion0\":{\"anger\":1.0740634E-05,\"contempt\":1.42603E-05,\"disgust\":5.26589356E-05,\"fear\":1.20671484E-06,\"happiness\":0.996862,\"neutral\":0.00302408962,\"sadness\":5.683868E-06,\"surprise\":2.93453577E-05},\"_expires\":\"2017-06-17T18:59:22.2148365Z\"},{\"Tags\":{\"T. J. Miller\":0.999,\"Silicon Valley\":0.999,\"Whitney\":0.264,\"Twitter Inc.\":1,\"HBO\":1,\"Complex\":0.032,\"The Gorburger Show\":0.062,\"Funny or Die\":0.914,\"Comedy Central\":1,\"Japan\":1,\"Ridiculousness\":0.004,\"Deadpool\":0.117,\"Cloverfield\":0.921,\"Cannes Film Festival\":0.987,\"Energizer Bunny\":0.007,\"Amy Schumer\":0.021,\"Pete Holmes\":0.153,\"Peter Boyle\":0.003,\"Downtown Los Angeles\":0.843,\"Supervillain\":0.038,\"San Francisco\":0.809,\"Jesus Christ\":0.902,\"Kong: Skull Island\":0.751,\"Jordan Vogt-Roberts\":0.979,\"Usher\":0.325,\"Flea\":0.536,\"Henry Rollins\":0.096,\"Federal government of the United States\":0.485,\"Mike Judge\":0.974,\"Uber\":0.404,\"Chelsea Handler\":0.364},\"_expires\":\"2017-06-17T18:59:22.7021704Z\"},{\"_expires\":\"2017-06-17T18:59:22.7489821Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/life/2017/06/iphone-8-edge-to-edge-screen\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/life/2017/06/iphone-8-edge-to-edge-screen\"},\"j\":[{\"_title\":\"Newly-Leaked Pictures Show You What iPhone 8 Screen Might Look Like\"},{\"RVisionTags\":{\"iPod\":0.802692235,\"electronics\":0.7313406},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0145561891,\"racyScore\":0.0151244327},\"_expires\":\"2017-06-17T23:14:19.5133399Z\"},{\"_expires\":\"2017-06-17T23:14:18.3277963Z\"},{\"Tags\":{\"iOS\":0.492,\"Complex\":0.102,\"Twitter Inc.\":1,\"Imgur\":0.996,\"Apple Inc.\":1,\"Reddit\":0.57,\"Check It Out\":0.003,\"China\":0.813,\"iPhone\":1},\"_expires\":\"2017-06-17T23:14:18.0245382Z\"},{\"XSentiment\":0.9994883,\"_expires\":\"2017-06-17T23:14:18.7364306Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/ryan-destiny-get-sweaty\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/ryan-destiny-get-sweaty\"},\"j\":[{\"_title\":\"Ryan Destiny Talks About Starring in Hit TV Series 'Star' on Get Sweaty With Emily Oberg\"},{\"RVisionTags\":{\"person\":0.998844266,\"boxing\":0.573501348},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0180808976,\"racyScore\":0.07170816},\"_expires\":\"2017-06-17T19:55:45.6659091Z\"},{\"Emotion0\":{\"anger\":0.0428811572,\"contempt\":0.0120455148,\"disgust\":0.00588818826,\"fear\":0.0031699785,\"happiness\":0.0325962044,\"neutral\":0.839067638,\"sadness\":0.0560076348,\"surprise\":0.00834368449},\"Emotion1\":{\"anger\":9.522394E-08,\"contempt\":3.42922242E-08,\"disgust\":3.1531672E-06,\"fear\":4.24115054E-09,\"happiness\":0.999994,\"neutral\":2.3093894E-06,\"sadness\":1.65964408E-07,\"surprise\":2.82413E-07},\"_expires\":\"2017-06-17T19:55:44.6508371Z\"},{\"Tags\":{\"Destiny\":0.299,\"HiT TV\":0.008,\"Complex\":0.953,\"Twitter Inc.\":1,\"New York City\":1,\"Robert E. Lee\":0.004,\"Fox Broadcasting Company\":1,\"Naomi Campbell\":0.091,\"Sy Kravitz\":0.731,\"Queen Latifah\":0.876},\"_expires\":\"2017-06-17T19:55:44.2710467Z\"},{\"XSentiment\":0.113136955,\"_expires\":\"2017-06-17T19:55:45.0269667Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\"},\"j\":[{\"_title\":\"Watch 2 Chainz Flex on Kendrick Lamar's \\\"DNA\\\" Beat in New Freestyle\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/everyday-struggle-ep39-kehlani-tinashe\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/everyday-struggle-ep39-kehlani-tinashe\"},\"j\":[{\"_title\":\"Joe Budden and DJ Akademiks Discuss Tinashe Controversy and Kehlani Cussing Out Heckler on 'Everyday Struggle'\"},{\"RVisionTags\":{\"abstract\":0.5319324},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.09698458,\"racyScore\":0.08712957},\"_expires\":\"2017-06-17T17:56:47.5354099Z\"},{\"Emotion0\":{\"anger\":0.0121765705,\"contempt\":0.0152475182,\"disgust\":0.0186378732,\"fear\":0.00241010683,\"happiness\":0.202662408,\"neutral\":0.6867056,\"sadness\":0.0549196824,\"surprise\":0.00724024652},\"Emotion1\":{\"anger\":0.005693211,\"contempt\":0.0003093396,\"disgust\":0.000264122762,\"fear\":0.000122387908,\"happiness\":0.000162528377,\"neutral\":0.9901545,\"sadness\":0.00110488839,\"surprise\":0.00218904181},\"_expires\":\"2017-06-17T17:56:46.5115225Z\"},{\"Tags\":{\"Joe Budden\":0.972,\"Disc jockey\":0.998,\"Tinashe\":0.485,\"Kehlani\":0.011,\"Profanity\":0.006,\"Heckler\":0.003,\"Complex\":0.992,\"Twitter Inc.\":0.959,\"XXL\":0.906,\"Kyrie Irving\":0.277,\"LeBron James\":0.998,\"Michael Jordan\":0.989,\"Floyd Mayweather Jr.\":0.031,\"Conor McGregor\":0.276},\"_expires\":\"2017-06-17T17:56:46.0896119Z\"},{\"XSentiment\":0.005011654,\"_expires\":\"2017-06-17T17:56:46.6990201Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/lonzo-ball-interview\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/lonzo-ball-interview\"},\"j\":[{\"_title\":\"Lonzo Ball Finally Told Us How He Really Feels About LaVar's Media Antics\"},{\"RVisionTags\":{\"person\":0.9992661,\"sport\":0.9894762,\"athletic game\":0.9814596,\"basketball\":0.9540427,\"player\":0.897939742,\"crowd\":0.529209554,\"watching\":0.450753957},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0182933789,\"racyScore\":0.0168850645},\"_expires\":\"2017-06-17T14:11:20.108671Z\"},{\"Emotion0\":{\"anger\":6.044781E-05,\"contempt\":2.85142833E-05,\"disgust\":4.95703644E-05,\"fear\":0.008861069,\"happiness\":2.874653E-05,\"neutral\":0.8961255,\"sadness\":0.00528799742,\"surprise\":0.08955817},\"_expires\":\"2017-06-17T14:11:19.6899531Z\"},{\"Tags\":{\"Los Angeles Angels of Anaheim\":0.515,\"Songwriter\":0.33,\"Twitter Inc.\":1,\"USA Today\":1,\"Broadcasting of sports events\":0.038,\"Chino Hills\":0.002,\"UCLA Bruins men's basketball\":0.6,\"NCAA Men's Division I Basketball Championship\":0.897,\"Sweet\":0.019,\"Todd Marinovich\":0.938,\"Marv Albert\":0.055,\"UNK NBA\":1,\"Fox Broadcasting Company\":0.933,\"Jayson Tatum\":0.004,\"ZO2\":0.008,\"Los Angeles Lakers\":1,\"Lamar Odom\":0.986,\"Lamar Cardinals Men's Basketball\":0.004,\"Magic Johnson\":0.995,\"Jason Kidd\":1,\"LeBron James\":1,\"James Harden\":0.99,\"Adidas\":0.811,\"Puerto Rico\":0.012,\"Stephen Curry\":0.981,\"Michael Jordan\":1,\"Shaquille O'Neal\":1},\"_expires\":\"2017-06-17T14:11:19.6587216Z\"},{\"XSentiment\":1,\"_expires\":\"2017-06-17T14:11:20.0892306Z\"}]}]},\"p\":[0.8153846,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154],\"VWState\":{\"m\":\"decc63fa2c284ec9887ee0572ea16d17/7860031216114d8bb718c40abc801bf4\"}}";

            using (var vw = new VowpalWabbit("--cb_adf"))
            {
                var obj = JsonConvert.DeserializeObject(json);
                var bytes = new byte[Encoding.UTF8.GetMaxByteCount(json.Length) + 1];
                var bytes2 = new byte[Encoding.UTF8.GetMaxByteCount(json.Length) + 1];
                var byteLen = Encoding.UTF8.GetBytes(json, 0, json.Length, bytes, 0) + 1;// trailing \0
                Array.Copy(bytes, bytes2, bytes.Length);
                VowpalWabbitDecisionServiceInteractionHeader header;
                List<VowpalWabbitExample> examples = null;

                try
                {
                    examples = vw.ParseDecisionServiceJson(bytes, 0, byteLen, copyJson: false, header: out header);

                    Assert.AreEqual("73369b13ec98433096a1496d27da0bfd", header.EventId);
                    CollectionAssert.AreEqual(new[] { 9, 11, 13, 6, 4, 5, 12, 1, 2, 10, 8, 3, 7 }, header.Actions, "Actions mismatch");
                    CollectionAssert.AreEqual(new[] { 0.8153846f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f }, header.Probabilities, "Probabilities mismatch");
                    Assert.AreEqual(0, header.ProbabilityOfDrop);

                    Assert.AreEqual(14, examples.Count);

                    // check if copyJson: false was actually used
                    CollectionAssert.AreNotEqual(bytes2, bytes);
                }
                finally
                {
                    if (examples != null)
                        foreach (var ex in examples)
                            if (ex != null)
                                ex.Dispose();
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestDecisionServiceJson_CopyJson()
        {
            var json = "{\"Version\":\"2\",\"EventId\":\"73369b13ec98433096a1496d27da0bfd\",\"a\":[9,11,13,6,4,5,12,1,2,10,8,3,7],\"c\":{\"_synthetic\":false,\"User\":{\"_age\":0},\"Geo\":{\"country\":\"United States\",\"_countrycf\":\"8\",\"state\":\"New Jersey\",\"city\":\"Somerdale\",\"_citycf\":\"5\",\"dma\":\"504\"},\"MRefer\":{\"referer\":\"http://www.complex.com/\"},\"OUserAgent\":{\"_ua\":\"Mozilla/5.0 (iPhone; CPU iPhone OS 10_3_2 like Mac OS X) AppleWebKit/603.2.4 (KHTML, like Gecko) Version/10.0 Mobile/14F89 Safari/602.1\",\"_DeviceBrand\":\"Apple\",\"_DeviceFamily\":\"iPhone\",\"_DeviceIsSpider\":false,\"_DeviceModel\":\"iPhone\",\"_OSFamily\":\"iOS\",\"_OSMajor\":\"10\",\"_OSPatch\":\"2\",\"DeviceType\":\"Mobile\"},\"_multi\":[{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweater-conor-mcgregor-may-be-set\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweater-conor-mcgregor-may-be-set\"},\"j\":[{\"_title\":\"The Floyd Mayweather vs. Conor McGregor Fight Date Has Finally Been Announced\"},{\"RVisionTags\":{\"person\":0.999368966,\"man\":0.998108864,\"wearing\":0.9368642,\"hat\":0.928866565,\"indoor\":0.893332958,\"close\":0.201101109},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0148094837,\"racyScore\":0.0144806523},\"_expires\":\"2017-06-17T21:42:30.3186957Z\"},{\"Emotion0\":{\"anger\":1.81591489E-07,\"contempt\":9.946987E-06,\"disgust\":6.11135547E-05,\"fear\":4.565633E-12,\"happiness\":0.999928534,\"neutral\":2.28114558E-07,\"sadness\":3.07409E-09,\"surprise\":1.46155665E-08},\"_expires\":\"2017-06-17T21:42:28.8496462Z\"},{\"Tags\":{\"Floyd Mayweather Jr.\":0.982,\"Conor McGregor\":0.938,\"Complex\":0.334,\"Twitter Inc.\":0.997,\"Dan Mullane\":0.006,\"Mixed martial arts\":1,\"Net Controls\":0.281,\"Boxing\":1,\"Ontario\":1,\"Dana White\":0.972,\"Las Vegas Valley\":0.995,\"Nevada Athletic Commission\":0.024,\"Mayweather Promotions\":0.118,\"MGM Grand Garden Arena\":0.997,\"Fighting game\":0.641,\"Nevada\":1,\"Coming out\":0.076},\"_expires\":\"2017-06-17T21:42:29.8271823Z\"},{\"XSentiment\":2.93618323E-05,\"_expires\":\"2017-06-17T21:42:29.1777863Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/young-thug-beautiful-thugger-girls-violent-trailer-has-people-upset\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/young-thug-beautiful-thugger-girls-violent-trailer-has-people-upset\"},\"j\":[{\"_title\":\"Why Young Thug's Violent Trailer for 'Beautiful Thugger Girls' Has People Upset\"},{\"RVisionTags\":{\"person\":0.9621564,\"indoor\":0.93759197},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0590519942,\"racyScore\":0.0740057454},\"_expires\":\"2017-06-17T17:27:41.0750729Z\"},{\"Emotion0\":{\"anger\":0.000195411936,\"contempt\":0.0007970728,\"disgust\":7.29157255E-05,\"fear\":0.000106336483,\"happiness\":0.000127831052,\"neutral\":0.9209777,\"sadness\":0.0775285438,\"surprise\":0.000194183245},\"_expires\":\"2017-06-17T17:27:33.4982953Z\"},{\"Tags\":{\"Young Thug\":0.445,\"Beautiful\":0.005,\"Twitter Inc.\":1,\"Drake\":0.968,\"Instagram\":0.995,\"Breezy\":0.014,\"June 13\":0.008,\"Cover art\":0.003,\"Album\":0.99,\"Surface\":0.012,\"Prince Michael Jackson II\":1},\"_expires\":\"2017-06-17T17:27:33.4670277Z\"},{\"XSentiment\":2.46002031E-07,\"_expires\":\"2017-06-17T17:27:34.0139318Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/did-we-just-witness-peak-lebron\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/did-we-just-witness-peak-lebron\"},\"j\":[{\"_title\":\"Did We Just Witness Peak LeBron?\"},{\"RVisionTags\":{\"person\":0.9887383,\"indoor\":0.8961688},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0181511845,\"racyScore\":0.0380923077},\"_expires\":\"2017-06-17T14:46:55.4499648Z\"},{\"Emotion0\":{\"anger\":0.0007875018,\"contempt\":0.000600368367,\"disgust\":0.00288117747,\"fear\":0.00168624625,\"happiness\":4.59727671E-05,\"neutral\":0.377137423,\"sadness\":0.00264490047,\"surprise\":0.6142164},\"_expires\":\"2017-06-17T14:46:49.7072998Z\"},{\"Tags\":{\"LeBron James\":1,\"Complex\":0.08,\"Broadcasting of sports events\":0.12,\"Twitter Inc.\":0.999,\"USA Today\":0.994,\"Kyle Broflovski\":0.023,\"Superman\":0.694,\"Cleveland Cavaliers\":1,\"UNK NBA\":1,\"Bill Russell NBA Finals Most Valuable Player Award\":1,\"Golden State Warriors\":1,\"Michael Jordan\":1,\"Kobe Bryant\":1,\"Time\":0.865,\"Magic Johnson\":0.999},\"_expires\":\"2017-06-17T14:46:49.4416286Z\"},{\"XSentiment\":0.9999974,\"_expires\":\"2017-06-17T14:46:49.7541575Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-made-the-difference\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-made-the-difference\"},\"j\":[{\"_title\":\"Golden State (Probably) Would Have Blown Another Lead Without KD\"},{\"RVisionTags\":{\"person\":0.999950767,\"player\":0.984833837,\"sport\":0.9816471,\"athletic game\":0.9691899,\"basketball\":0.7260069,\"hand\":0.463383943,\"crowd\":0.3354485},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0127091147,\"racyScore\":0.01585682},\"_expires\":\"2017-06-17T14:44:47.7452593Z\"},{\"Emotion0\":{\"anger\":0.9830929,\"contempt\":6.38814454E-05,\"disgust\":0.0142428661,\"fear\":4.3443215E-05,\"happiness\":0.00127731345,\"neutral\":0.0003242454,\"sadness\":0.0006526729,\"surprise\":0.0003026811},\"_expires\":\"2017-06-17T14:44:47.1857099Z\"},{\"Tags\":{\"Golden State Warriors\":1,\"Lead guitar\":0.046,\"Kevin Durant\":1,\"Twitter Inc.\":0.985,\"Martinez\":0.015,\"Splash Brothers\":1,\"Cleveland Cavaliers\":1,\"Monday Night Football\":0.398,\"Richard Jefferson\":0.95,\"Kevin Love\":0.997,\"McDonald's All-American Game\":0.859,\"University of Texas at Austin College of Fine Arts\":0.999,\"Naismith College Player of the Year\":0.117,\"UNK NBA\":1,\"NBA Most Valuable Player Award\":1,\"Olympic Games\":0.858,\"NBA All-Star Game\":1,\"Champion\":0.12,\"Bill Russell NBA Finals Most Valuable Player Award\":1,\"Stephen Curry\":0.989,\"Draymond Green\":0.674,\"Seat Pleasant\":0.122},\"_expires\":\"2017-06-17T14:44:46.9264793Z\"},{\"XSentiment\":1,\"_expires\":\"2017-06-17T14:44:47.4639789Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\"},\"j\":[{\"_title\":\"Watch 2 Chainz Flex on Kendrick Lamar's \\\"DNA\\\" Beat in New Freestyle\"},{\"RVisionTags\":{\"person\":0.9840661,\"dressed\":0.304036647},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0285988934,\"racyScore\":0.0257010516},\"_expires\":\"2017-06-17T19:30:46.5524171Z\"},{\"_expires\":\"2017-06-17T19:30:46.2086571Z\"},{\"Tags\":{\"2 Chainz\":1,\"Kendrick Lamar\":1,\"DNA\":0.053,\"Hip hop production\":0.015,\"Freestyle rap\":0.934,\"Philadelphia\":0.193,\"Twitter Inc.\":1,\"Subscription business model\":0.011,\"Complex\":1,\"Los Angeles\":1,\"Georgia\":0.894,\"Trap\":0.579,\"Top Dawg Entertainment\":0.872,\"Virtual reality\":0.011,\"Travis Scott\":0.011,\"Collaboration\":0.004,\"Nicki Minaj\":0.999,\"Remy Ma\":0.99,\"Papoose\":0.892,\"Sampling\":0.017,\"Hip hop music\":1,\"Tha Carter V\":1,\"Everyday (ASAP Rocky song)\":0.003},\"_expires\":\"2017-06-17T19:30:46.1314041Z\"},{\"XSentiment\":1,\"_expires\":\"2017-06-17T19:30:46.7944713Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-viral-challenge-backfires-reacts\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/floyd-mayweather-viral-challenge-backfires-reacts\"},\"j\":[{\"_title\":\"Floyd Mayweather Attempted to Start His Own Viral Challenge and It Hilariously Backfired\"},{\"RVisionTags\":{\"person\":0.999924064,\"man\":0.9552084,\"crowd\":0.01707872},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0114686647,\"racyScore\":0.0159121435},\"_expires\":\"2017-06-17T21:13:01.1925796Z\"},{\"Emotion0\":{\"anger\":0.00127403683,\"contempt\":0.0222781487,\"disgust\":0.0002176114,\"fear\":9.893078E-06,\"happiness\":0.07823167,\"neutral\":0.897131145,\"sadness\":0.00048493495,\"surprise\":0.0003725856},\"_expires\":\"2017-06-17T21:12:59.3669891Z\"},{\"Tags\":{\"Floyd Mayweather Jr.\":0.192,\"Philadelphia\":0.65,\"Twitter Inc.\":1,\"USA Today\":0.979,\"Sport\":0.85,\"Conor McGregor\":0.016,\"June 14\":0.009,\"Troy\":0.005,\"Honda Civic\":0.004,\"Bank account\":0.025,\"NASCAR on TNT\":0.099,\"Boxing\":1},\"_expires\":\"2017-06-17T21:12:59.3201526Z\"},{\"XSentiment\":0.999998,\"_expires\":\"2017-06-17T21:13:01.7647699Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-discusses-random-text-he-received-from-obama-after-winning-nba-finals\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/kevin-durant-discusses-random-text-he-received-from-obama-after-winning-nba-finals\"},\"j\":[{\"_title\":\"Kevin Durant Discusses 'Random' Text He Received From Obama After Winning NBA Finals\"},{\"RVisionTags\":{\"person\":0.9939494,\"outdoor\":0.9162231,\"male\":0.242890328},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0208446719,\"racyScore\":0.0304868072},\"_expires\":\"2017-06-17T18:28:04.5675692Z\"},{\"Emotion0\":{\"anger\":2.35385942E-05,\"contempt\":3.294205E-06,\"disgust\":1.38025935E-05,\"fear\":7.8729E-06,\"happiness\":0.9997165,\"neutral\":0.0001607666,\"sadness\":1.25915176E-05,\"surprise\":6.161377E-05},\"Emotion1\":{\"anger\":0.00371822272,\"contempt\":0.000460597221,\"disgust\":0.000157746123,\"fear\":0.000275517668,\"happiness\":0.0403934456,\"neutral\":0.86978966,\"sadness\":0.08499769,\"surprise\":0.000207107121},\"_expires\":\"2017-06-17T18:28:03.3944386Z\"},{\"Tags\":{\"Kevin Durant\":1,\"Random House\":0.069,\"Barack Obama\":1,\"UNK NBA\":1,\"Twitter Inc.\":1,\"USA Today\":0.999,\"Sports journalism\":0.01,\"Cary\":0.025,\"The NBA Finals\":0.017,\"Monday Night Football\":0.943,\"Golden State Warriors\":1,\"Bill Simmons\":0.866,\"Podcast\":0.918,\"Oracle Arena\":0.028,\"Bill Russell NBA Finals Most Valuable Player Award\":1,\"June 13\":0.034,\"Cleveland Cavaliers\":1,\"LeBron James\":1,\"Kyrie Irving\":1,\"Allen Iverson\":0.999,\"Rihanna\":0.63,\"The League\":0.166,\"Stay\":0.858,\"Singing\":0.024,\"President of the United States\":1},\"_expires\":\"2017-06-17T18:28:03.8094333Z\"},{\"XSentiment\":0.9999663,\"_expires\":\"2017-06-17T18:28:03.9031587Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/pop-culture/2017/06/tj-miller-hbo-special-interview\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/pop-culture/2017/06/tj-miller-hbo-special-interview\"},\"j\":[{\"_title\":\"T.J. Miller's Done With 'Silicon Valley,' But His Career's Just Getting Started\"},{\"RVisionTags\":{\"person\":0.999560535,\"man\":0.9939658,\"suit\":0.950625,\"outdoor\":0.9169477,\"wearing\":0.7708228,\"jacket\":0.528340757,\"coat\":0.490623325,\"dark\":0.32463637,\"male\":0.212570518,\"microphone\":0.148984566,\"crowd\":0.0156043554},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0102083739,\"racyScore\":0.0126593616},\"_expires\":\"2017-06-17T18:59:23.2248419Z\"},{\"Emotion0\":{\"anger\":1.0740634E-05,\"contempt\":1.42603E-05,\"disgust\":5.26589356E-05,\"fear\":1.20671484E-06,\"happiness\":0.996862,\"neutral\":0.00302408962,\"sadness\":5.683868E-06,\"surprise\":2.93453577E-05},\"_expires\":\"2017-06-17T18:59:22.2148365Z\"},{\"Tags\":{\"T. J. Miller\":0.999,\"Silicon Valley\":0.999,\"Whitney\":0.264,\"Twitter Inc.\":1,\"HBO\":1,\"Complex\":0.032,\"The Gorburger Show\":0.062,\"Funny or Die\":0.914,\"Comedy Central\":1,\"Japan\":1,\"Ridiculousness\":0.004,\"Deadpool\":0.117,\"Cloverfield\":0.921,\"Cannes Film Festival\":0.987,\"Energizer Bunny\":0.007,\"Amy Schumer\":0.021,\"Pete Holmes\":0.153,\"Peter Boyle\":0.003,\"Downtown Los Angeles\":0.843,\"Supervillain\":0.038,\"San Francisco\":0.809,\"Jesus Christ\":0.902,\"Kong: Skull Island\":0.751,\"Jordan Vogt-Roberts\":0.979,\"Usher\":0.325,\"Flea\":0.536,\"Henry Rollins\":0.096,\"Federal government of the United States\":0.485,\"Mike Judge\":0.974,\"Uber\":0.404,\"Chelsea Handler\":0.364},\"_expires\":\"2017-06-17T18:59:22.7021704Z\"},{\"_expires\":\"2017-06-17T18:59:22.7489821Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/life/2017/06/iphone-8-edge-to-edge-screen\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/life/2017/06/iphone-8-edge-to-edge-screen\"},\"j\":[{\"_title\":\"Newly-Leaked Pictures Show You What iPhone 8 Screen Might Look Like\"},{\"RVisionTags\":{\"iPod\":0.802692235,\"electronics\":0.7313406},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0145561891,\"racyScore\":0.0151244327},\"_expires\":\"2017-06-17T23:14:19.5133399Z\"},{\"_expires\":\"2017-06-17T23:14:18.3277963Z\"},{\"Tags\":{\"iOS\":0.492,\"Complex\":0.102,\"Twitter Inc.\":1,\"Imgur\":0.996,\"Apple Inc.\":1,\"Reddit\":0.57,\"Check It Out\":0.003,\"China\":0.813,\"iPhone\":1},\"_expires\":\"2017-06-17T23:14:18.0245382Z\"},{\"XSentiment\":0.9994883,\"_expires\":\"2017-06-17T23:14:18.7364306Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/ryan-destiny-get-sweaty\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/ryan-destiny-get-sweaty\"},\"j\":[{\"_title\":\"Ryan Destiny Talks About Starring in Hit TV Series 'Star' on Get Sweaty With Emily Oberg\"},{\"RVisionTags\":{\"person\":0.998844266,\"boxing\":0.573501348},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0180808976,\"racyScore\":0.07170816},\"_expires\":\"2017-06-17T19:55:45.6659091Z\"},{\"Emotion0\":{\"anger\":0.0428811572,\"contempt\":0.0120455148,\"disgust\":0.00588818826,\"fear\":0.0031699785,\"happiness\":0.0325962044,\"neutral\":0.839067638,\"sadness\":0.0560076348,\"surprise\":0.00834368449},\"Emotion1\":{\"anger\":9.522394E-08,\"contempt\":3.42922242E-08,\"disgust\":3.1531672E-06,\"fear\":4.24115054E-09,\"happiness\":0.999994,\"neutral\":2.3093894E-06,\"sadness\":1.65964408E-07,\"surprise\":2.82413E-07},\"_expires\":\"2017-06-17T19:55:44.6508371Z\"},{\"Tags\":{\"Destiny\":0.299,\"HiT TV\":0.008,\"Complex\":0.953,\"Twitter Inc.\":1,\"New York City\":1,\"Robert E. Lee\":0.004,\"Fox Broadcasting Company\":1,\"Naomi Campbell\":0.091,\"Sy Kravitz\":0.731,\"Queen Latifah\":0.876},\"_expires\":\"2017-06-17T19:55:44.2710467Z\"},{\"XSentiment\":0.113136955,\"_expires\":\"2017-06-17T19:55:45.0269667Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/2-chainz-dna-freestyle-kendrick-lamar\"},\"j\":[{\"_title\":\"Watch 2 Chainz Flex on Kendrick Lamar's \\\"DNA\\\" Beat in New Freestyle\"}]},{\"_tag\":\"cmplx$http://www.complex.com/music/2017/06/everyday-struggle-ep39-kehlani-tinashe\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/music/2017/06/everyday-struggle-ep39-kehlani-tinashe\"},\"j\":[{\"_title\":\"Joe Budden and DJ Akademiks Discuss Tinashe Controversy and Kehlani Cussing Out Heckler on 'Everyday Struggle'\"},{\"RVisionTags\":{\"abstract\":0.5319324},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.09698458,\"racyScore\":0.08712957},\"_expires\":\"2017-06-17T17:56:47.5354099Z\"},{\"Emotion0\":{\"anger\":0.0121765705,\"contempt\":0.0152475182,\"disgust\":0.0186378732,\"fear\":0.00241010683,\"happiness\":0.202662408,\"neutral\":0.6867056,\"sadness\":0.0549196824,\"surprise\":0.00724024652},\"Emotion1\":{\"anger\":0.005693211,\"contempt\":0.0003093396,\"disgust\":0.000264122762,\"fear\":0.000122387908,\"happiness\":0.000162528377,\"neutral\":0.9901545,\"sadness\":0.00110488839,\"surprise\":0.00218904181},\"_expires\":\"2017-06-17T17:56:46.5115225Z\"},{\"Tags\":{\"Joe Budden\":0.972,\"Disc jockey\":0.998,\"Tinashe\":0.485,\"Kehlani\":0.011,\"Profanity\":0.006,\"Heckler\":0.003,\"Complex\":0.992,\"Twitter Inc.\":0.959,\"XXL\":0.906,\"Kyrie Irving\":0.277,\"LeBron James\":0.998,\"Michael Jordan\":0.989,\"Floyd Mayweather Jr.\":0.031,\"Conor McGregor\":0.276},\"_expires\":\"2017-06-17T17:56:46.0896119Z\"},{\"XSentiment\":0.005011654,\"_expires\":\"2017-06-17T17:56:46.6990201Z\"}]},{\"_tag\":\"cmplx$http://www.complex.com/sports/2017/06/lonzo-ball-interview\",\"i\":{\"constant\":1,\"id\":\"cmplx$http://www.complex.com/sports/2017/06/lonzo-ball-interview\"},\"j\":[{\"_title\":\"Lonzo Ball Finally Told Us How He Really Feels About LaVar's Media Antics\"},{\"RVisionTags\":{\"person\":0.9992661,\"sport\":0.9894762,\"athletic game\":0.9814596,\"basketball\":0.9540427,\"player\":0.897939742,\"crowd\":0.529209554,\"watching\":0.450753957},\"SVisionAdult\":{\"isAdultContent\":false,\"isRacyContent\":false,\"adultScore\":0.0182933789,\"racyScore\":0.0168850645},\"_expires\":\"2017-06-17T14:11:20.108671Z\"},{\"Emotion0\":{\"anger\":6.044781E-05,\"contempt\":2.85142833E-05,\"disgust\":4.95703644E-05,\"fear\":0.008861069,\"happiness\":2.874653E-05,\"neutral\":0.8961255,\"sadness\":0.00528799742,\"surprise\":0.08955817},\"_expires\":\"2017-06-17T14:11:19.6899531Z\"},{\"Tags\":{\"Los Angeles Angels of Anaheim\":0.515,\"Songwriter\":0.33,\"Twitter Inc.\":1,\"USA Today\":1,\"Broadcasting of sports events\":0.038,\"Chino Hills\":0.002,\"UCLA Bruins men's basketball\":0.6,\"NCAA Men's Division I Basketball Championship\":0.897,\"Sweet\":0.019,\"Todd Marinovich\":0.938,\"Marv Albert\":0.055,\"UNK NBA\":1,\"Fox Broadcasting Company\":0.933,\"Jayson Tatum\":0.004,\"ZO2\":0.008,\"Los Angeles Lakers\":1,\"Lamar Odom\":0.986,\"Lamar Cardinals Men's Basketball\":0.004,\"Magic Johnson\":0.995,\"Jason Kidd\":1,\"LeBron James\":1,\"James Harden\":0.99,\"Adidas\":0.811,\"Puerto Rico\":0.012,\"Stephen Curry\":0.981,\"Michael Jordan\":1,\"Shaquille O'Neal\":1},\"_expires\":\"2017-06-17T14:11:19.6587216Z\"},{\"XSentiment\":1,\"_expires\":\"2017-06-17T14:11:20.0892306Z\"}]}]},\"p\":[0.8153846,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154],\"VWState\":{\"m\":\"decc63fa2c284ec9887ee0572ea16d17/7860031216114d8bb718c40abc801bf4\"}}";

            using (var vw = new VowpalWabbit("--cb_adf"))
            {
                var obj = JsonConvert.DeserializeObject(json);
                var bytes = new byte[Encoding.UTF8.GetMaxByteCount(json.Length) + 1];
                var bytes2 = new byte[Encoding.UTF8.GetMaxByteCount(json.Length) + 1];
                var byteLen = Encoding.UTF8.GetBytes(json, 0, json.Length, bytes, 0) + 1;// trailing \0
                Array.Copy(bytes, bytes2, bytes.Length);
                VowpalWabbitDecisionServiceInteractionHeader header;
                List<VowpalWabbitExample> examples = null;

                try
                {
                    examples = vw.ParseDecisionServiceJson(bytes, 0, byteLen, copyJson: true, header: out header);

                    Assert.AreEqual("73369b13ec98433096a1496d27da0bfd", header.EventId);
                    CollectionAssert.AreEqual(new[] { 9, 11, 13, 6, 4, 5, 12, 1, 2, 10, 8, 3, 7 }, header.Actions, "Actions mismatch");
                    CollectionAssert.AreEqual(new[] { 0.8153846f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f }, header.Probabilities, "Probabilities mismatch");
                    Assert.AreEqual(0, header.ProbabilityOfDrop);

                    Assert.AreEqual(14, examples.Count);

                    // check if copyJson: true was actually used
                    CollectionAssert.AreEqual(bytes2, bytes);
                }
                finally
                {
                    if (examples != null)
                        foreach (var ex in examples)
                            if (ex != null)
                                ex.Dispose();
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestDecisionServiceJsonNull()
        {
            var json = @"{""Version"":""1"",""EventId"":""7cacacea2c6e49b5b922f6f517a325ed"",""a"":[9,4,13,10,8,5,2,3,12,11,7,6,1],""c"":{""_synthetic"":false,""User"":{""_age"":0},""Geo"":{""country"":""United States"",""_countrycf"":""8"",""state"":""Georgia"",""city"":""Stone Mountain"",""_citycf"":""5"",""dma"":""524""},""MRefer"":{""referer"":""http://www.complex.com/""},""OUserAgent"":{""_ua"":""Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_5) AppleWebKit/603.2.4 (KHTML, like Gecko) Version/10.1.1 Safari/603.2.4"",""_DeviceBrand"":"""",""_DeviceFamily"":""Other"",""_DeviceIsSpider"":false,""_DeviceModel"":"""",""_OSFamily"":""Mac OS X"",""_OSMajor"":""10"",""_OSPatch"":""5"",""DeviceType"":""Desktop""},""_multi"":[{""_tag"":""cmplx$http://www.complex.com/music/2017/06/prodigy-mobb-deep-once-in-a-generation-rapper"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/music/2017/06/prodigy-mobb-deep-once-in-a-generation-rapper""},""j"":[{""_title"":""Why Prodigy Was A Once-In-A-Generation Rapper""},{""RVisionTags"":{""person"":0.9913805,""hat"":0.6433856,""male"":0.153918922},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.0162594952,""racyScore"":0.0152094},""TVisionCelebrities"":{""Prodigy"":0.9999119},""_expires"":""2017-06-24T22:43:00.9241929Z""},{""Emotion0"":{""anger"":0.005261584,""contempt"":0.01940289,""disgust"":0.00146069494,""fear"":7.486289E-05,""happiness"":0.0102698216,""neutral"":0.9544214,""sadness"":0.00859182,""surprise"":0.00051694276},""_expires"":""2017-06-24T22:43:00.0008415Z""},{""Tags"":{""Roc Marciano"":0.015,""Mobb Deep"":1,""Prodigy"":1,""Havoc"":1,""A Tribe Called Quest"":0.969,""Q-Tip"":0.992,""The Infamous"":1,""The Crystals"":0.01,""Fifth Beatle"":0.099},""_expires"":""2017-06-24T22:43:00.1727355Z""},{""XSentiment"":3.01036973E-13,""_expires"":""2017-06-24T22:43:00.9398236Z""}]},{""_tag"":""cmplx$http://www.complex.com/pop-culture/2017/06/best-movies-2017"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/pop-culture/2017/06/best-movies-2017""},""j"":[{""_title"":""The Best Movies of 2017 (So Far)""},{""RVisionTags"":{""text"":0.9992092,""book"":0.997986436},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.0431842171,""racyScore"":0.0587232448},""_expires"":""2017-06-25T03:08:01.2111373Z""},{""Emotion0"":{""anger"":0.005728849,""contempt"":0.00117533945,""disgust"":8.215821E-05,""fear"":1.32827354E-05,""happiness"":0.000487715733,""neutral"":0.9911194,""sadness"":0.000120451317,""surprise"":0.00127282448},""_expires"":""2017-06-25T03:08:00.8636383Z""},null,{""XSentiment"":1,""_expires"":""2017-06-25T03:08:01.5112947Z""}]},{""_tag"":""cmplx$http://www.complex.com/music/2017/06/rihanna-responds-to-dm-from-fan-seeking-advice-on-getting-over-his-first-heartbreak"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/music/2017/06/rihanna-responds-to-dm-from-fan-seeking-advice-on-getting-over-his-first-heartbreak""},""j"":[{""_title"":""Rihanna Responds to DM From Fan Seeking Advice on Getting Over His First Heartbreak""},{""RVisionTags"":{""person"":0.99633044},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.013516671,""racyScore"":0.048148632},""_expires"":""2017-06-25T02:01:20.2363494Z""},{""Emotion0"":{""anger"":3.085194E-06,""contempt"":0.000411317043,""disgust"":1.026042E-05,""fear"":3.766979E-07,""happiness"":0.9783716,""neutral"":0.0211082119,""sadness"":5.66137569E-05,""surprise"":3.850023E-05},""_expires"":""2017-06-25T02:01:19.7050726Z""},{""Tags"":{""Rihanna"":1,""Martinez"":0.021,""Complex"":0.834,""Twitter Inc."":1,""If You"":0.003,""Grammy Awards"":1,""Mathematics"":0.999,""Malawi"":0.298,""Christine Teigen"":0.009,""Dave Chappelle"":0.047,""Presenter"":0.892,""Kendrick Lamar"":0.984,""Diamonds"":0.998},""_expires"":""2017-06-25T02:01:19.4155039Z""},{""XSentiment"":1,""_expires"":""2017-06-25T02:01:24.9467431Z""}]},{""_tag"":""cmplx$http://www.complex.com/music/2017/06/rihanna-responds-to-dm-from-fan-seeking-advice-on-getting-over-his-first-heartbreak"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/music/2017/06/rihanna-responds-to-dm-from-fan-seeking-advice-on-getting-over-his-first-heartbreak""},""j"":[{""_title"":""Rihanna Responds to DM From Fan Seeking Advice on Getting Over His First Heartbreak""}]},{""_tag"":""cmplx$http://www.complex.com/life/2017/06/guy-changes-from-shorts-to-dress-after-getting-sent-home-from-work-on-hot-day"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/life/2017/06/guy-changes-from-shorts-to-dress-after-getting-sent-home-from-work-on-hot-day""},""j"":[{""_title"":""Guy Sent Home by Boss for Wearing Shorts on a Hot Day, Returns to Work in Mom's Dress""},{""RVisionTags"":{""person"":0.999545038,""indoor"":0.998509467,""wall"":0.997952163},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.034187492,""racyScore"":0.0335518233},""_expires"":""2017-06-24T21:27:46.7135793Z""},{""Emotion0"":{""anger"":7.7880286E-05,""contempt"":0.00116215891,""disgust"":4.65850326E-06,""fear"":3.797253E-06,""happiness"":2.82076635E-05,""neutral"":0.986993253,""sadness"":0.0116343917,""surprise"":9.564323E-05},""_expires"":""2017-06-24T21:27:46.1197986Z""},{""Tags"":{""Boss Corporation"":0.007,""Shorts"":0.139,""Complex"":0.169,""Twitter Inc."":1,""English"":0.603,""The Daily Mirror"":0.008,""Oklahoma"":0.948,""High school"":0.999,""Lists of National Basketball Association players"":0.072,""UNK NBA"":0.529,""NBA dress code"":0.031,""Dress code"":0.113},""_expires"":""2017-06-24T21:27:46.0729078Z""},{""XSentiment"":0.9995655,""_expires"":""2017-06-24T21:27:46.7448359Z""}]},{""_tag"":""cmplx$http://www.complex.com/pop-culture/2017/06/tom-cruise-allegedly-balanced-bible-study-and-blow-jobs-away-from-risky-business-set"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/pop-culture/2017/06/tom-cruise-allegedly-balanced-bible-study-and-blow-jobs-away-from-risky-business-set""},""j"":[{""_title"":""Tom Cruise Was Allegedly Balancing Bible Study and Blow Jobs Away From the 'Risky Business' Set""},{""RVisionTags"":{""person"":0.9992724,""man"":0.9368908},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.009141326,""racyScore"":0.00951602},""TVisionCelebrities"":{""TOM CRUISE"":0.999997854},""_expires"":""2017-06-24T22:43:00.578981Z""},{""Emotion0"":{""anger"":1.77455458E-05,""contempt"":0.000173967157,""disgust"":0.000229260724,""fear"":3.15066657E-08,""happiness"":0.9786317,""neutral"":0.0209334176,""sadness"":5.861598E-07,""surprise"":1.33144977E-05},""_expires"":""2017-06-24T22:43:00.0793561Z""},{""Tags"":{""Connor Cruise"":1,""The Bible"":1,""Risky Business"":0.999,""Martinez"":0.006,""Complex"":0.01,""Twitter Inc."":1,""Sean Penn"":0.989,""Curtis Armstrong"":0.625,""The Hollywood Reporter"":0.203,""Louis Armstrong"":0.348,""Cruise ship"":1,""Chicago"":0.997,""Rebecca De Mornay"":0.434,""Christianity"":0.75,""James Corden"":0.069,""Hollywood"":1,""Stunt"":0.45},""_expires"":""2017-06-24T22:43:00.0008415Z""},{""XSentiment"":0.178008512,""_expires"":""2017-06-24T22:43:01.2750814Z""}]},{""_tag"":""cmplx$http://www.complex.com/music/2017/06/goldlink-releases-crew-remix-featuring-gucci-mane"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/music/2017/06/goldlink-releases-crew-remix-featuring-gucci-mane""},""j"":[{""_title"":""Gucci Mane Jumps on Remix of GoldLink's \""Crew\""""},{""RVisionTags"":{""text"":0.9904163},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.140676036,""racyScore"":0.08041213},""_expires"":""2017-06-25T03:29:03.5393778Z""},{""Emotion0"":{""anger"":0.09716808,""contempt"":0.00540762255,""disgust"":0.0220363177,""fear"":0.0168951228,""happiness"":0.00362249953,""neutral"":0.772761941,""sadness"":0.028037779,""surprise"":0.05407063},""Emotion1"":{""anger"":0.419406265,""contempt"":0.032055337,""disgust"":0.03494472,""fear"":0.004095346,""happiness"":0.218424827,""neutral"":0.2713901,""sadness"":0.0143981427,""surprise"":0.005285231},""_expires"":""2017-06-25T03:29:01.7568819Z""},{""Tags"":{""Gucci Mane"":0.993,""Goldlink"":0.246,""Joshua"":0.003,""Twitter Inc."":0.987,""Public Relations"":0.054,""Washington, D.C."":1,""Shy Glizzy"":0.149,""Turntablism"":0.004,""Shazam"":0.915,""Apple Music"":0.473,""You Can"":0.003,""iTunes"":0.956,""United States"":1,""Country"":0.996,""Miami Heat"":0.888,""Houston Rockets"":0.81,""Los Angeles"":1,""Portland Trail Blazers"":0.972,""Brooklyn Nets"":0.74,""Philadelphia"":1,""Monument Records"":0.173,""Go-go"":0.558,""Down"":0.126,""Album"":0.984,""Xavier Musketeers men's basketball"":0.011,""TEAM*"":0.006,""No Way Out"":0.007},""_expires"":""2017-06-25T03:29:01.4924568Z""},{""XSentiment"":0.7643385,""_expires"":""2017-06-25T03:29:02.1167452Z""}]},{""_tag"":""cmplx$http://www.complex.com/music/2017/06/snoop-dogg-mock-young-thug-and-lil-uzi-vert-moment-i-feared-video"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/music/2017/06/snoop-dogg-mock-young-thug-and-lil-uzi-vert-moment-i-feared-video""},""j"":[{""_title"":""It Looks Like Snoop Dogg Is Mocking Young Thug and Lil Uzi Vert in New Video for \""Moment I Feared\""""},{""RVisionTags"":{""person"":0.998487234},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.0242747813,""racyScore"":0.02104937},""_expires"":""2017-06-24T20:58:27.8900749Z""},{""Emotion0"":{""anger"":0.000228447025,""contempt"":0.0007477614,""disgust"":0.000145930273,""fear"":1.614125E-07,""happiness"":0.00138306723,""neutral"":0.997234,""sadness"":0.000220693677,""surprise"":3.99426281E-05},""_expires"":""2017-06-24T20:58:27.5944467Z""},{""Tags"":{""Snoop Dogg"":1,""Young Thug"":0.959,""Uzi"":0.67,""Vert (music producer)"":0.003,""New Video"":0.003,""Kyle Broflovski"":0.032,""Philadelphia"":0.006,""Twitter Inc."":1,""Subscription business model"":0.008,""Complex"":0.995,""WorldStarHipHop"":0.237,""Rick Rock"":0.449,""Fonzie"":0.026,""Hyphy"":0.228,""YouTube"":0.999,""300 Entertainment"":0.009,""I Do"":0.006,""Billboard Music Award for Woman of the Year"":0.007,""You Know What It Is"":0.007,""Beautiful"":1,""Hip hop music"":1},""_expires"":""2017-06-24T20:58:27.1518034Z""},{""XSentiment"":0.0003323581,""_expires"":""2017-06-24T20:58:28.2026657Z""}]},{""_tag"":""cmplx$http://www.complex.com/sports/2017/06/will-lebron-james-ever-be-the-goat-square-up"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/sports/2017/06/will-lebron-james-ever-be-the-goat-square-up""},""j"":[{""_title"":""Square Up: Will LeBron James Ever Be the G.O.A.T.?""},{""RVisionTags"":{""person"":0.996526659,""outdoor"":0.8773945,""player"":0.8584581,""athletic game"":0.8081653,""sport"":0.7153003,""green"":0.640901864},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.0165802147,""racyScore"":0.0130834375},""_expires"":""2017-06-24T17:04:57.3297536Z""},{""Emotion0"":{""anger"":0.03943785,""contempt"":0.05141066,""disgust"":0.0008398856,""fear"":3.766839E-05,""happiness"":0.00017841009,""neutral"":0.9013937,""sadness"":0.00586816063,""surprise"":0.0008336294},""_expires"":""2017-06-24T17:04:55.650471Z""},{""Tags"":{""Square, Inc."":0.005,""LeBron James"":1,""Complex"":0.926,""Twitter Inc."":0.991,""UNK NBA"":1,""Lil B"":0.415,""Cleveland Cavaliers"":1,""Golden State Warriors"":1},""_expires"":""2017-06-24T17:04:54.617989Z""},{""XSentiment"":0.00127977342,""_expires"":""2017-06-24T17:04:55.400445Z""}]},{""_tag"":""cmplx$http://www.complex.com/life/2017/06/cop-who-killed-philando-castile-says-smell-of-weed-made-him-fear-for-life"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/life/2017/06/cop-who-killed-philando-castile-says-smell-of-weed-made-him-fear-for-life""},""j"":[{""_title"":""Cop Who Killed Philando Castile Says Smell of Weed Made Him Fear for His Life""},{""RVisionTags"":{""sky"":0.998623848,""outdoor"":0.980991364},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.01796771,""racyScore"":0.0286844},""_expires"":""2017-06-24T16:33:36.9062355Z""},{""_expires"":""2017-06-24T16:33:36.2030935Z""},{""Tags"":{""Scared"":0.011,""Philadelphia"":0.693,""Twitter Inc."":1,""Ramsey County"":0.995,""County attorney"":0.084,""Minnesota"":1,""The Life of the Party"":0.009,""And I"":0.003,""Police"":0.238,""Dashcam"":0.092,""Murder"":0.673,""Complex"":0.508},""_expires"":""2017-06-24T16:33:35.8124592Z""},{""XSentiment"":4.440892E-16,""_expires"":""2017-06-24T16:33:36.4218429Z""}]},{""_tag"":""cmplx$http://www.complex.com/music/2017/06/a-history-of-bow-wow-fails"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/music/2017/06/a-history-of-bow-wow-fails""},""j"":[{""_title"":""A History of Bow Wow Taking L's""},{""RVisionTags"":{""person"":0.999946833,""crowd"":0.236136854},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.02204849,""racyScore"":0.0223767031},""_expires"":""2017-06-24T15:56:22.5001293Z""},{""Emotion0"":{""anger"":1.22613847E-05,""contempt"":0.0200665444,""disgust"":0.000140431745,""fear"":8.001536E-09,""happiness"":0.883786,""neutral"":0.0959461853,""sadness"":1.50497419E-06,""surprise"":4.703166E-05},""_expires"":""2017-06-24T15:56:21.7295682Z""},{""Tags"":{""Bow Wow"":1,""He Is"":0.003,""Forbes"":0.995,""Hip hop music"":1,""iTunes"":0.974,""Atlantic Ocean"":0.051,""Vibe"":0.982,""The Source"":0.991,""GQ"":1,""Esquire"":0.998,""Stephen Sondheim"":0.606,""Twitter Inc."":1,""Snoop Dogg"":1,""The Arsenio Hall Show"":0.043,""Kurtis Blow"":0.325,""Michael Jordan"":0.785,""Vine"":0.585,""Oh, hell"":0.005,""Rent"":0.155,""Los Angeles"":1,""Grammy Awards"":1,""Scuderia Ferrari"":0.099,""Instagram"":0.996,""Migos"":0.01,""December 8"":0.003,""Live television"":0.401,""Grammy Awards Ceremony"":0.102,""Timothy Sykes"":0.173,""President of the United States"":0.998,""Donald Trump"":0.069,""Pacific Time Zone"":0.278,""Funkmaster Flex"":0.375,""Jermaine Dupri"":0.996,""Complex"":0.959},""_expires"":""2017-06-24T15:56:22.1957887Z""},{""XSentiment"":1,""_expires"":""2017-06-24T15:56:22.336416Z""}]},{""_tag"":""cmplx$http://www.complex.com/style/2017/06/how-rhude-one-of-best-los-angeles-brands-started-with-a-single-t-shirt"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/style/2017/06/how-rhude-one-of-best-los-angeles-brands-started-with-a-single-t-shirt""},""j"":[{""_title"":""How Rhude, One of the Best L.A. Brands, Started With a Single T-Shirt ""},{""RVisionTags"":{""person"":0.993622959,""floor"":0.989562333,""indoor"":0.98370254,""standing"":0.810940444,""curtain"":0.7955723,""suit"":0.335544765},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.08148283,""racyScore"":0.0423882678},""_expires"":""2017-06-24T12:04:37.9596756Z""},{""Emotion0"":{""anger"":0.0006152864,""contempt"":0.00599214761,""disgust"":0.0002703283,""fear"":1.61605785E-05,""happiness"":0.00174113235,""neutral"":0.984110057,""sadness"":0.006996317,""surprise"":0.0002585811},""Emotion1"":{""anger"":3.47017163E-07,""contempt"":0.00013010086,""disgust"":2.59245485E-06,""fear"":1.34914092E-06,""happiness"":0.000740572,""neutral"":0.997228861,""sadness"":0.00182852661,""surprise"":6.765087E-05},""_expires"":""2017-06-24T12:04:37.3269255Z""},{""Tags"":{""The Best"":0.013,""Starting pitcher"":0.489,""T-shirt"":0.041,""Twitter Inc."":0.955,""Mixmaster Spade"":0.977,""Kendrick Lamar"":1,""Snoop Dogg"":1,""BET Awards"":0.014,""ASAP Rocky"":0.979,""Kevin Durant"":0.023,""Jimmy Butler"":0.052,""Barneys New York"":0.011,""Patron saint"":0.034,""United States"":1,""Big Sean"":0.986,""Manila"":0.676,""Culture of the Philippines"":0.938,""Trade in Services Agreement"":0.006,""Kanye West"":1,""Arnold Schwarzenegger"":0.004,""Comme des Garçons"":0.987,""Rei Kawakubo"":0.911,""Sugar Land"":0.671,""Texas"":1,""Posttraumatic stress disorder"":0.02,""Earth, Wind & Fire"":0.003,""Mike Jones"":0.007,""White American"":0.012,""Prada Marfa"":1,""Elmgreen and Dragset"":0.992,""Complex"":0.992},""_expires"":""2017-06-24T12:04:36.8672077Z""},{""XSentiment"":1,""_expires"":""2017-06-24T12:04:37.5488193Z""}]},{""_tag"":""cmplx$http://www.complex.com/music/2017/06/bow-wow-gets-backlash-over-sexist-instagram-caption"",""i"":{""constant"":1,""id"":""cmplx$http://www.complex.com/music/2017/06/bow-wow-gets-backlash-over-sexist-instagram-caption""},""j"":[{""_title"":""The Internet Blasts Bow Wow for His Message About Women's Behavior""},{""RVisionTags"":{""person"":0.962055564,""man"":0.9547968},""SVisionAdult"":{""isAdultContent"":false,""isRacyContent"":false,""adultScore"":0.0112607479,""racyScore"":0.0106668333},""TVisionCelebrities"":{""Bow Wow"":0.9601661},""_expires"":""2017-06-24T01:43:32.5645689Z""},{""Emotion0"":{""anger"":0.0011651055,""contempt"":0.005131879,""disgust"":0.000264819653,""fear"":2.01475978E-05,""happiness"":0.000101240934,""neutral"":0.98257935,""sadness"":0.0105332062,""surprise"":0.000204226963},""_expires"":""2017-06-24T01:43:31.8143804Z""},{""Tags"":{""The Internet"":0.661,""Bow Wow"":0.398,""Twitter Inc."":1,""Instagram"":0.998,""I Wonder Why"":0.004,""Sexism"":0.362},""_expires"":""2017-06-24T01:43:31.5472109Z""},{""XSentiment"":0.99999994,""_expires"":""2017-06-24T01:43:32.2439044Z""}]}]},""p"":[0.8153846,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154,0.0153846154],""VWState"":{""m"":""4c5bcf8eb1ef4d3ba327dacba2f336a1/4c5bcf8eb1ef4d3ba327dacba2f336a1""}}"; 

            using (var vw = new VowpalWabbit("--cb_adf"))
            {
                var obj = JsonConvert.DeserializeObject(json);
                var bytes = new byte[Encoding.UTF8.GetMaxByteCount(json.Length) + 1];
                var byteLen = Encoding.UTF8.GetBytes(json, 0, json.Length, bytes, 0) + 1;// trailing \0
                VowpalWabbitDecisionServiceInteractionHeader header;
                List<VowpalWabbitExample> examples = null;

                try
                {
                    examples = vw.ParseDecisionServiceJson(bytes, 0, byteLen, copyJson: false, header: out header);

                    Assert.AreEqual("7cacacea2c6e49b5b922f6f517a325ed", header.EventId);
                    CollectionAssert.AreEqual(new[] { 9, 4, 13, 10, 8, 5, 2, 3, 12, 11, 7, 6, 1 }, header.Actions, "Actions mismatch");
                    CollectionAssert.AreEqual(new[] { 0.8153846f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f, 0.0153846154f }, header.Probabilities, "Probabilities mismatch");
                    Assert.AreEqual(0, header.ProbabilityOfDrop);

                    Assert.AreEqual(14, examples.Count);
                }
                finally
                {
                    if (examples != null)
                        foreach (var ex in examples)
                            if (ex != null)
                                ex.Dispose();
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/JSON")]
        public void TestPartialJson()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator(new VowpalWabbitSettings
            {
                Arguments = "--cb_adf",
                PropertyConfiguration = new PropertyConfiguration
                {
                    MultiProperty = "adf",
                    TextProperty = "someText",
                    FeatureIgnorePrefix = "xxx"
                }
            }))
            {
                var json = "{\"eventId\":123,\"c\":{\"Age\":25,\"adf\":[{\"someText\":\"w1 w2\", \"a\":{\"x\":1}, \"xxxxIgnoreMe\":2}, {\"someText\":\"w2 w3\"}], \"_labelIndex\":1, \"_label_Cost\":-1, \"_label_Probability\":0.3},\"post\":456}";
                using (var textReader = new JsonTextReader(new StringReader(json)))
                {
                    textReader.Read();
                    Assert.AreEqual(JsonToken.StartObject, textReader.TokenType);

                    textReader.Read();
                    Assert.AreEqual(JsonToken.PropertyName, textReader.TokenType);
                    Assert.AreEqual("eventId", textReader.Value);
                    textReader.Read();
                    Assert.AreEqual(JsonToken.Integer, textReader.TokenType);
                    Assert.AreEqual((Int64)123, textReader.Value);

                    textReader.Read();
                    Assert.AreEqual(JsonToken.PropertyName, textReader.TokenType);
                    Assert.AreEqual("c", textReader.Value);
                    textReader.Read();

                    validator.Validate(new[] {
                         "shared | Age:25",
                         " | w1 w2 |a x:1",
                         "0:-1:.3 | w2 w3"
                        },
                        textReader,
                        VowpalWabbitLabelComparator.ContextualBandit);

                    textReader.Read();
                    Assert.AreEqual(JsonToken.PropertyName, textReader.TokenType);
                    Assert.AreEqual("post", textReader.Value);
                    textReader.Read();
                    Assert.AreEqual((Int64)456, textReader.Value);
                }
            }
        }

        public class MyContext
        {
            [Feature]
            public int Feature { get; set; }

            [JsonProperty("_multi")]
            public IEnumerable<MyADF> Multi { get; set; }
        }

        public class MyADF
        {
            [Feature]
            public int Foo { get; set; }
        }

        [TestMethod]
        public void TestNumADFs()
        {
            var jsonDirectSerializer = VowpalWabbitSerializerFactory.CreateSerializer<MyContext>(new VowpalWabbitSettings { TypeInspector = JsonTypeInspector.Default })
                as IVowpalWabbitMultiExampleSerializerCompiler<MyContext>;

            Assert.IsNotNull(jsonDirectSerializer);
            Assert.AreEqual(3,
                jsonDirectSerializer.GetNumberOfActionDependentExamples(new MyContext { Multi = new MyADF[3] }));
        }
    }
}
