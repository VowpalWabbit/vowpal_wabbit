using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Serializer;
using VW.Serializer.Attributes;
using VW.Serializer.Interfaces;
using VW.Serializer.Intermediate;
using VW.Serializer.Reflection;
using VW.Serializer.Visitors;

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
        private VowpalWabbit vw;

        private VowpalWabbitInterfaceVisitor visitor;

        public CustomFeaturizer(VowpalWabbit vw, VowpalWabbitInterfaceVisitor visitor)
        {
            this.vw = vw;
            this.visitor = visitor;
        }

        public void Visit(IFeature<CustomClass> feature)
        {
            Assert.AreEqual(5, feature.Value.X);
            Assert.IsNotNull(this.vw);
            Assert.IsNotNull(this.visitor);

            feature.Value.HasVisited = true;
        }
    }

    [TestClass]
    public class TestSerializer
    {
        [TestMethod]
        public void TestCustomFeaturizer()
        {
            var serializer = VowpalWabbitSerializerFactory.CreateSerializer<MyContext>(new VW.VowpalWabbitSettings(), customFeaturizer: new List<Type> { typeof(CustomFeaturizer) });

            var context = new MyContext() { Feature = new CustomClass() { X = 5 }};
            using (var vw = new VowpalWabbit(""))
            {
                var example = serializer.Serialize(vw, context);

                Assert.IsNotNull(example);

                example.Dispose();
            }

            Assert.IsTrue(context.Feature.HasVisited);
        }

        [TestMethod]
        public void TestCustomFeaturizerOverideMethod()
        {
            var serializer = VowpalWabbitSerializerFactory.CreateSerializer<MyContext>(new VW.VowpalWabbitSettings(), new List<FeatureExpression>
            {
                new FeatureExpression(typeof(CustomClass), "Feature", 
                    // TODO: looks a bit awkward for an API. The compiler needs to know what property to access to copy the value into the Feature<T> object
                    valueExpression => Expression.Property(valueExpression, (PropertyInfo)ReflectionHelper.GetInfo((MyContext m) => m.Feature)),
                    overrideSerializeMethod: (MethodInfo)ReflectionHelper.GetInfo((CustomFeaturizer c) => c.Visit(null)))
            });

            var context = new MyContext() { Feature = new CustomClass() { X = 5 } };
            using (var vw = new VowpalWabbit(""))
            {
                var example = serializer.Serialize(vw, context);

                Assert.IsNotNull(example);

                example.Dispose();
            }

            Assert.IsTrue(context.Feature.HasVisited);
        }
    }
}
