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


        /// <summary>
        /// Regression test for #4945. SimpleLabelUpdateExample wrote the importance weight to
        /// example::weight, but VW::setup_example -- which the builder runs inside CreateExample --
        /// reassigns that field from the simple-label reduction features, so the weight was
        /// overwritten with the default of 1 before anything could read it back. StringLabel was
        /// unaffected because the text parser writes the reduction feature directly.
        /// </summary>
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestSimpleLabelRetainsWeight()
        {
            using (var vw = new VowpalWabbit("--quiet"))
            using (var builder = new VowpalWabbitExampleBuilder(vw))
            {
                builder.ApplyLabel(new SimpleLabel { Label = 7f, Weight = 4f, Initial = 3f });

                using (var example = builder.CreateExample())
                {
                    var read = (SimpleLabel)example.Label;
                    Assert.AreEqual(7f, read.Label, "label");
                    Assert.AreEqual(4f, read.Weight, "weight");
                    Assert.AreEqual(3f, read.Initial, "initial");
                }
            }
        }

        /// <summary>
        /// The consequence of the above: a weight that never reaches the learner trains as though
        /// it were 1.0, so these two produced identical predictions before the fix.
        /// </summary>
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestSimpleLabelWeightAffectsTraining()
        {
            Func<ILabel, float> train = label =>
            {
                using (var vw = new VowpalWabbit("--link logistic --loss_function logistic -b 18 --quiet"))
                {
                    using (var builder = new VowpalWabbitExampleBuilder(vw))
                    {
                        using (var ns = builder.AddNamespace('a'))
                        {
                            ns.AddFeature(vw.HashFeature("x", vw.HashSpace("a")), 1f);
                        }

                        builder.ApplyLabel(label);

                        using (var example = builder.CreateExample())
                        {
                            vw.Learn(example);
                        }
                    }

                    using (var scoreBuilder = new VowpalWabbitExampleBuilder(vw))
                    {
                        using (var ns = scoreBuilder.AddNamespace('a'))
                        {
                            ns.AddFeature(vw.HashFeature("x", vw.HashSpace("a")), 1f);
                        }

                        using (var scored = scoreBuilder.CreateExample())
                        {
                            return vw.Predict(scored, VowpalWabbitPredictionType.Scalar);
                        }
                    }
                }
            };

            var heavy = train(new SimpleLabel { Label = 1f, Weight = 4f });
            var light = train(new SimpleLabel { Label = 1f, Weight = 0.05f });

            Assert.AreNotEqual(heavy, light,
                "importance weight had no effect on training: both behaved as weight 1.0");
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
