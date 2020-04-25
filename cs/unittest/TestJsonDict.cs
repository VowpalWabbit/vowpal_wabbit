using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading;
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
            public Context(float[] vector, int id, JsonSerializerSettings settings)
            {
                this.Vector = vector;
                this.Id = id;

                this.JSON = JsonConvert.SerializeObject(this, settings);

                var sb = new StringBuilder();
                sb.AppendFormat("| Id:{0}", this.Id);

                foreach (var v in vector)
                    sb.AppendFormat(CultureInfo.InvariantCulture, " :{0}", v);

                this.VW = sb.ToString();
            }

            [JsonProperty(IsReference = true)]
            public float[] Vector { get; private set; }

            [JsonProperty]
            public int Id { get; private  set; }

            [JsonIgnore]
            public string VW { get; set; }

            [JsonIgnore]
            public string JSON { get; set; }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestJsonDict()
        {
            var vec = new float[] { 1, 2, 3 };

            var jsonResolver = new RefResolve();
            var settings = new JsonSerializerSettings { ReferenceResolverProvider = () => jsonResolver };

            var ctx1 = new Context(vec, 1, settings);
            var ctx2 = new Context(vec, 2, settings);

            using (var vw = new VowpalWabbit(new VowpalWabbitSettings { EnableStringExampleGeneration = true }))
            using (var resolver = new VowpalWabbitJsonReferenceResolver(serializer => Assert.Fail()))
            using (var serializer1 = new VowpalWabbitJsonSerializer(vw, resolver))
            using (var example1 = serializer1.ParseAndCreate(ctx1.JSON))
            using (var serializer2 = new VowpalWabbitJsonSerializer(vw, resolver))
            using (var example2 = serializer2.ParseAndCreate(ctx2.JSON))
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                validator.Validate("| Id:1 :1 :2 :3", example1);
                validator.Validate(ctx1.VW, example1);
                validator.Validate("| Id:2 :1 :2 :3", example2);
                validator.Validate(ctx2.VW, example2);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestJsonDictReverse()
        {
            var vec = new float[] { 1, 2, 3 };

            var jsonResolver = new RefResolve();
            var settings = new JsonSerializerSettings { ReferenceResolverProvider = () => jsonResolver };

            var ctx1 = new Context(vec, 1, settings);
            var ctx2 = new Context(vec, 2, settings);

            VowpalWabbitJsonSerializer delayedSerializer = null;

            using (var validator = new VowpalWabbitExampleJsonValidator())
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings { EnableStringExampleGeneration = true }))
            using (var resolver = new VowpalWabbitJsonReferenceResolver(serializer => delayedSerializer = serializer))
            {
                var serializer2 = new VowpalWabbitJsonSerializer(vw, resolver);
                var example2 = serializer2.ParseAndCreate(ctx2.JSON);

                // incomplete data
                Assert.IsNull(example2);

                // triggers example2 completion
                using (var serializer1 = new VowpalWabbitJsonSerializer(vw, resolver))
                using (var example1 = serializer1.ParseAndCreate(ctx1.JSON))
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

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestJsonDictThreading()
        {
            var jsonResolver = new RefResolve();
            var settings = new JsonSerializerSettings { ReferenceResolverProvider = () => jsonResolver };

            var rnd = new Random(123);
            var examples = new List<Context>();

            var id = 0;
            // different reference objects
            for (int i = 0; i < 10; i++)
            {
                var data = Enumerable.Range(1, 5).Select(_ => (float)rnd.Next(10)).ToArray();

                // referencing the same data
                for (int j = 0; j < 5; j++)
                    examples.Add(new Context(data, id++, settings));
            }

            for (int i = 0; i < 4; i++)
            {
                Permute(examples, rnd);

                for (int maxDegreeOfParallelism = 1; maxDegreeOfParallelism < 4; maxDegreeOfParallelism++)
			    {
                    var examplesFound = 0;
                    using (var vw = new VowpalWabbit(new VowpalWabbitSettings { EnableStringExampleGeneration = true, EnableThreadSafeExamplePooling = true }))
                    using (var resolver = new VowpalWabbitJsonReferenceResolver(serializer =>
                        {
                            using (var example = serializer.CreateExamples())
                            {
                                ValidateExample(example, (Context)serializer.UserContext);
                            }

                            serializer.Dispose();

                            Interlocked.Increment(ref examplesFound);
                        }))
                    {
                        Parallel.ForEach(
                            Partitioner.Create(0, examples.Count),
                            new ParallelOptions { MaxDegreeOfParallelism = maxDegreeOfParallelism },
                            range =>
                            {
                                for (int j = range.Item1; j < range.Item2; j++)
                                {
                                    var ctx = examples[j];
                                    var serializer = new VowpalWabbitJsonSerializer(vw, resolver) { UserContext = ctx };

                                    var example = serializer.ParseAndCreate(ctx.JSON);

                                    // example not ready yet
                                    if (example == null)
                                        continue;

                                    ValidateExample(example, ctx);

                                    example.Dispose();
                                    serializer.Dispose();

                                    Interlocked.Increment(ref examplesFound);
                                }
                            });
                    }

                    Assert.AreEqual(examples.Count, examplesFound);
                }
            }
        }

        public void ValidateExample(VowpalWabbitExampleCollection example, Context ctx)
        {
            using (var vw = new VowpalWabbit(new VowpalWabbitSettings { EnableStringExampleGeneration = true }))
            using (var validator = new VowpalWabbitExampleJsonValidator())
            {
                var singleExample = (VowpalWabbitSingleLineExampleCollection)example;
                validator.Validate(ctx.VW, example);
            }
        }


        public static void Permute<T>(List<T> arr, Random rnd)
        {
            for (int i = 0; i < arr.Count - 1; i++)
            {
                int swapIndex = rnd.Next(i, arr.Count);

                T temp = arr[swapIndex];
                arr[swapIndex] = arr[i];
                arr[i] = temp;
            }
        }

        public class RefResolve : IReferenceResolver
        {
            private readonly IDictionary<object, object> data;
            private readonly IDictionary<string, object> otherData;

            private class ReferenceEqualityComparer : IEqualityComparer<object>
            {
                bool IEqualityComparer<object>.Equals(object x, object y)
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
