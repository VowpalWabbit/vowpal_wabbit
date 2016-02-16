using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
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
    public class TestJsonClass
    {
        [TestMethod]
        [TestCategory("JSON")]
        public void TestJson()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1}}");
                validator.Validate("|a foo:2.3", "{\"a\":{\"foo\":2.3}}");
                validator.Validate("|a foo:2.3 bar", "{\"a\":{\"foo\":2.3, \"bar\":true}}");
                validator.Validate("|a foo:1 |bcd Age25_old", "{\"a\":{\"foo\":1},\"bcd\":{\"Age\":\"25 old\"}}");
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonAux()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":{\"abc\":{\"def\":3}}}");
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":5}");
                validator.Validate("|a foo:1", "{\"a\":{\"foo\":1},\"_aux\":[1,2,[3,4],2]}");
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonArray()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("| :1 :2.3 :4", "{\"a\":[1,2.3,4]}");
                validator.Validate("|a :1 :2.3 :4", "{\"a\":{\"b\":[1,2.3,4]}}");
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonSimpleLabel()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("1 |a foo:1", "{\"_label\":{\"Label\":1},\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonVWLabel()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("1 |a foo:1", "{\"_label\":1,\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
                validator.Validate("1 |a foo:1", "{\"_label\":\"1\",\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple);
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonSimpleLabelOverride()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("2 |a foo:1", "{\"_label\":{\"Label\":1},\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.Simple,
                    new SimpleLabel { Label = 2 });
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonContextualBanditLabel()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator("--cb 2 --cb_type dr"))
            {
                validator.Validate("1:-2:.3 |a foo:1",
                    "{\"_label\":{\"Action\":1,\"Cost\":-2,\"Probability\":.3},\"a\":{\"foo\":1}}",
                    VowpalWabbitLabelComparator.ContextualBandit);
                validator.Validate("1:2:.5 |a foo:1", "{\"_label\":\"1:2:.5\",\"a\":{\"foo\":1}}", VowpalWabbitLabelComparator.ContextualBandit);
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
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
                    DontConsider = "XXX"
                },
                Ns2 = new Namespace2
                {
                    FeatureA = true
                },
                Clicks = 5
            };

            var jsonContextString = JsonConvert.SerializeObject(jsonContext);
            using (var validator = new VowpalWabbitExampleJsonValidator(""))
            {
                validator.Validate("25 |a Bar:1 Age25 |b Marker | Clicks:5 MoreClicks:0",
                    jsonContextString,
                    VowpalWabbitLabelComparator.Simple);
            }
        }

        [TestMethod]
        [TestCategory("JSON")]
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
        }

        [TestMethod]
        [TestCategory("JSON")]
        public void TestJsonText()
        {
            using (var validator = new VowpalWabbitExampleJsonValidator(""))
            {
                validator.Validate("| a b c |a d e f", "{\"_text\":\"a b c\",\"a\":{\"_text\":\"d e f\"}}");
            }
        }
    }
}
