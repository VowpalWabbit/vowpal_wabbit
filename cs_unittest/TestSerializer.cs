using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Reflection;
using VW.Serializer;
using VW.Serializer.Attributes;
using VW.Serializer.Intermediate;

namespace cs_unittest
{
    public class CustomClass
    {
        public int X { get; set; }

        public bool HasVisited = false;
    }

    public class MyContext
    {
        [Feature]
        public CustomClass Feature { get; set; }
    }

    public class CustomFeaturizer
    {
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, CustomClass value)
        {
            Assert.IsNotNull(context);
            Assert.IsNotNull(ns);
            Assert.IsNotNull(feature);
            Assert.IsNotNull(value);

            Assert.AreEqual(5, value.X);

            value.HasVisited = true;
        }
    }

    [TestClass]
    public class TestSerializer
    {
        [TestMethod]
        public void TestCustomFeaturizer()
        {

            var context = new MyContext() { Feature = new CustomClass() { X = 5 }};
            using (var vw = new VowpalWabbit(""))
            {
                var serializer = VowpalWabbitSerializerFactory.CreateSerializer<MyContext>(new VowpalWabbitSettings(customFeaturizer: new List<Type> { typeof(CustomFeaturizer) }))
                    .Create(vw);

                var example = serializer.Serialize(context);

                Assert.IsNotNull(example);

                example.Dispose();
            }

            Assert.IsTrue(context.Feature.HasVisited);
        }

        [TestMethod]
        public void TestCustomFeaturizerOverideMethod()
        {
            var context = new MyContext() { Feature = new CustomClass() { X = 5 } };
            using (var vw = new VowpalWabbit(""))
            {
                var serializer = VowpalWabbitSerializerFactory.CreateSerializer<MyContext>(new VowpalWabbitSettings(allFeatures: new List<FeatureExpression>
                {
                    new FeatureExpression(typeof(CustomClass), "Feature",
                        // TODO: looks a bit awkward for an API. The compiler needs to know what property to access to copy the value into the Feature<T> object
                        valueExpression => Expression.Property(valueExpression, (PropertyInfo)ReflectionHelper.GetInfo((MyContext m) => m.Feature)),
                        overrideSerializeMethod: (MethodInfo)ReflectionHelper.GetInfo((CustomFeaturizer c) => c.MarshalFeature(null, null, null, null)))
                })).Create(vw);
                var example = serializer.Serialize(context);

                Assert.IsNotNull(example);

                example.Dispose();
            }

            Assert.IsTrue(context.Feature.HasVisited);
        }
    }
}
