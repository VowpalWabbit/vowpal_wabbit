using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Serializer;

namespace cs_unittest
{
    [TestClass]
    public class TestJsonDictClass
    {
        public class Context
        {
            [JsonProperty(IsReference = true)]
            public float[] Vector { get; set; }

            [JsonProperty]
            public int Id { get; set; }
        }

        [TestMethod]
        public void TestJsonDict()
        {
            var vec = new float[] { 1, 2, 3 };

            var jsonResolver = new RefResolve();
            var settings = new JsonSerializerSettings { ReferenceResolverProvider = () => jsonResolver };

            var json1 = JsonConvert.SerializeObject(new Context { Id = 1, Vector = vec }, settings);
            var json2 = JsonConvert.SerializeObject(new Context { Id = 2, Vector = vec }, settings);

            using (var vw = new VowpalWabbit(new VowpalWabbitSettings(enableStringExampleGeneration: true)))
            using (var resolver = new VowpalWabbitJsonReferenceResolver(serializer => Assert.Fail()))
            using (var serializer1 = new VowpalWabbitJsonSerializer(vw, resolver))
            using (var example1 = serializer1.ParseAndCreate(json1))
            using (var serializer2 = new VowpalWabbitJsonSerializer(vw, resolver))
            using (var example2 = serializer2.ParseAndCreate(json2))
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("| Id:1 :1 :2 :3", example1);
                validator.Validate("| Id:2 :1 :2 :3", example2);
            }
        }

        [TestMethod]
        public void TestJsonDictReverse()
        {
            var vec = new float[] { 1, 2, 3 };

            var jsonResolver = new RefResolve();
            var settings = new JsonSerializerSettings { ReferenceResolverProvider = () => jsonResolver };

            var json1 = JsonConvert.SerializeObject(new Context { Id = 1, Vector = vec }, settings);
            var json2 = JsonConvert.SerializeObject(new Context { Id = 2, Vector = vec }, settings);

            VowpalWabbitJsonSerializer delayedSerializer = null;

            using (var validator = new VowpalWabbitExampleJsonValidator())
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings(enableStringExampleGeneration: true)))
            using (var resolver = new VowpalWabbitJsonReferenceResolver(serializer => delayedSerializer = serializer))
            {
                var serializer2 = new VowpalWabbitJsonSerializer(vw, resolver);
                var example2 = serializer2.ParseAndCreate(json2);

                // incomplete data
                Assert.IsNull(example2);

                // triggers example2 completion
                using (var serializer1 = new VowpalWabbitJsonSerializer(vw, resolver))
                using (var example1 = serializer1.ParseAndCreate(json1))
                {
                    validator.Validate("| Id:1 :1 :2 :3", example1);
                }

                Assert.IsNotNull(delayedSerializer);

                using (var delayedExample2 = delayedSerializer.CreateExamples())
                {
                    validator.Validate("| Id:2 :1 :2 :3", delayedExample2);
                }

                delayedSerializer.Dispose();
            }
        }

        public class RefResolve : IReferenceResolver
        {
            private readonly IDictionary<object, object> data;
            private readonly IDictionary<string, object> otherData;

            private class ReferenceEqualityComparer : IEqualityComparer<object>
            {
                public bool Equals(object x, object y)
                {
                    return object.ReferenceEquals(x, y);
                }

                public int GetHashCode(object obj)
                {
                    return obj.GetHashCode();
                }
            }

            public RefResolve()
            {
                this.data = new Dictionary<object, object>(new ReferenceEqualityComparer());
                this.otherData = new Dictionary<string, object>();
            }

            public object ResolveReference(object context, string reference)
            {
                return this.otherData[reference];
            }

            public string GetReference(object context, object value)
            {
                foreach (var kv in this.otherData)
                {
                    if (object.ReferenceEquals(kv.Value, value))
                        return kv.Key;
                }

                var id = Guid.NewGuid().ToString();

                this.AddReference(null, id, value);

                return id;
            }

            public bool IsReferenced(object context, object value)
            {
                return this.otherData.Any(kv => object.ReferenceEquals(kv.Value, value)) ||
                    this.data.ContainsKey(value);
            }

            public void AddReference(object context, string reference, object value)
            {
                this.otherData.Add(reference, value);
                this.data.Add(value, value);
            }
        }
    }
}
