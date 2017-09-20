using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer;
using VW.Serializer.Attributes;

namespace cs_unittest
{
    [TestClass]
    public class TestLabelsClass
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestLabels()
        {
            using (var vw = new VowpalWabbitExampleValidator<SimpleContext>(string.Empty))
            {
                vw.Validate("3.2 | Feature:25",
                    new SimpleContext
                    {
                        Feature = 25,
                        Label = new SimpleLabel { Label = 3.2f }
                    });

                vw.Validate("| Feature:25",
                    new SimpleContext
                    {
                        Feature = 25,
                    });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestLabelsNoAnnotation()
        {
            using (var vw = new VowpalWabbitExampleValidator<SimpleContextNoAnnotation>(
                new VowpalWabbitSettings { TypeInspector = TypeInspector.All }))
            {
                vw.Validate("3.2 | Feature:25",
                    new SimpleContextNoAnnotation
                    {
                        Feature = 25,
                        Label = new SimpleLabel { Label = 3.2f }
                    });

                vw.Validate("| Feature:25",
                    new SimpleContextNoAnnotation
                    {
                        Feature = 25,
                    });
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Marshal")]
        [TestCategory("Vowpal Wabbit")]
        public void TestStringLabels()
        {
            using (var vw = new VowpalWabbitExampleValidator<SimpleStringContext>(string.Empty))
            {
                vw.Validate("3.2 | Feature:25",
                    new SimpleStringContext
                    {
                        Feature = 25,
                        Label = "3.2"
                    });

                vw.Validate("| Feature:25",
                    new SimpleStringContext
                    {
                        Feature = 25,
                    });
            }
        }

    }

    public class SimpleContext
    {
        [Feature]
        public int Feature { get; set; }

        [Label]
        public ILabel Label { get; set; }
    }

    public class SimpleContextNoAnnotation
    {
        public int Feature { get; set; }

        public ILabel Label { get; set; }
    }
    public class SimpleStringContext
    {
        [Feature]
        public int Feature { get; set; }

        [Label]
        public string Label { get; set; }
    }
}
