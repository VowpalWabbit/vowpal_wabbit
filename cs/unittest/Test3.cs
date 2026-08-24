using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using TrainSet0002Dat;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class BuilderTestClass : TestBase
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/example builder")]
        public void TestBuilderSimple()
        {
          using (VowpalWabbit vw = new VowpalWabbit(""))
          {
            VowpalWabbitExample e;

            using (var exampleBuilder = new VowpalWabbitExampleBuilder(vw))
            using (var nsBuilder = exampleBuilder.AddNamespace('U'))
            {
                ulong nsHash = vw.HashSpace("User");
                nsBuilder.AddFeature(vw.HashFeature("e1", nsHash), 0.3425f);

                e = exampleBuilder.CreateExample();
            }

            Debug.Assert(e != null);
            foreach (var n in e)
            {
                Debug.WriteLine($"+ ({n.Index})=>'{(char)n.Index}'");
                foreach (var f in n)
                {
                    Debug.WriteLine($"-- {f.WeightIndex}:{f.X}");
                }
            }
          }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/example builder")]
        public void TestBuilderNamespaceVisibleBeforeDispose()
        {
            using (VowpalWabbit vw = new VowpalWabbit("--noconstant"))
            {
                ulong nsHash = vw.HashSpace("User");
                ulong featureHash = vw.HashFeature("e1", nsHash);

                // CreateExample() is called while the namespace builder is still open. The namespace has
                // to already be registered on the example, otherwise setup_example iterates right past it
                // and leaves the weight indices unscaled by the stride multiplier.
                VowpalWabbitExample eager;
                using (var exampleBuilder = new VowpalWabbitExampleBuilder(vw))
                using (var nsBuilder = exampleBuilder.AddNamespace('U'))
                {
                    nsBuilder.AddFeature(featureHash, 0.3425f);

                    eager = exampleBuilder.CreateExample();
                }

                VowpalWabbitExample deferred;
                using (var exampleBuilder = new VowpalWabbitExampleBuilder(vw))
                {
                    using (var nsBuilder = exampleBuilder.AddNamespace('U'))
                    {
                        nsBuilder.AddFeature(featureHash, 0.3425f);
                    }

                    deferred = exampleBuilder.CreateExample();
                }

                using (eager)
                using (deferred)
                {
                    Assert.AreEqual(1ul, eager.NumberOfFeatures);
                    Assert.IsNull(eager.Diff(vw, deferred, null));
                    Assert.AreEqual(deferred.Single().Single().WeightIndex, eager.Single().Single().WeightIndex);
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/example builder")]
        public void TestBuilderClone()
        {
            using (VowpalWabbit vw = new VowpalWabbit("--noconstant"))
            {
                ulong sharedHash = vw.HashSpace("Shared");
                ulong candidateHash = vw.HashSpace("Candidate");

                using (var template = new VowpalWabbitExampleBuilder(vw))
                {
                    using (var shared = template.AddNamespace('S'))
                    {
                        shared.AddFeature(vw.HashFeature("s1", sharedHash), 1f);
                        shared.AddFeature(vw.HashFeature("s2", sharedHash), 2f);
                    }

                    for (int i = 0; i < 2; i++)
                    {
                        VowpalWabbitExample cloned;
                        using (var builder = template.Clone())
                        {
                            using (var candidate = builder.AddNamespace('C'))
                            {
                                candidate.AddFeature(vw.HashFeature($"c{i}", candidateHash), 3f);
                            }

                            cloned = builder.CreateExample();
                        }

                        VowpalWabbitExample expected;
                        using (var builder = new VowpalWabbitExampleBuilder(vw))
                        {
                            using (var shared = builder.AddNamespace('S'))
                            {
                                shared.AddFeature(vw.HashFeature("s1", sharedHash), 1f);
                                shared.AddFeature(vw.HashFeature("s2", sharedHash), 2f);
                            }

                            using (var candidate = builder.AddNamespace('C'))
                            {
                                candidate.AddFeature(vw.HashFeature($"c{i}", candidateHash), 3f);
                            }

                            expected = builder.CreateExample();
                        }

                        using (cloned)
                        using (expected)
                        {
                            Assert.AreEqual(3ul, cloned.NumberOfFeatures);
                            Assert.IsNull(cloned.Diff(vw, expected, null));
                        }
                    }

                    // Cloning must leave the template intact, and must not carry anything the clones added.
                    using (var templateExample = template.CreateExample())
                    {
                        Assert.AreEqual(2ul, templateExample.NumberOfFeatures);
                        Assert.AreEqual((byte)'S', templateExample.Single().Index);
                    }
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/example builder")]
        public void TestBuilderAddFeatures()
        {
            using (VowpalWabbit vw = new VowpalWabbit("--noconstant"))
            {
                ulong nsHash = vw.HashSpace("User");
                ulong[] weightIndices = Enumerable.Range(0, 8).Select(i => vw.HashFeature($"e{i}", nsHash)).ToArray();
                float[] values = Enumerable.Range(0, 8).Select(i => (float)(i + 1)).ToArray();

                // 0-valued features are dropped, matching AddFeature.
                values[3] = 0f;

                VowpalWabbitExample batched;
                using (var builder = new VowpalWabbitExampleBuilder(vw))
                {
                    using (var ns = builder.AddNamespace('U'))
                    {
                        ns.AddFeatures(weightIndices, values);
                    }

                    batched = builder.CreateExample();
                }

                VowpalWabbitExample individual;
                using (var builder = new VowpalWabbitExampleBuilder(vw))
                {
                    using (var ns = builder.AddNamespace('U'))
                    {
                        for (int i = 0; i < weightIndices.Length; i++)
                        {
                            ns.AddFeature(weightIndices[i], values[i]);
                        }
                    }

                    individual = builder.CreateExample();
                }

                using (batched)
                using (individual)
                {
                    Assert.AreEqual(7ul, batched.NumberOfFeatures);
                    Assert.IsNull(batched.Diff(vw, individual, null));
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/example builder")]
        public void TestBuilderAddFeaturesPartialRange()
        {
            using (VowpalWabbit vw = new VowpalWabbit("--noconstant"))
            {
                ulong nsHash = vw.HashSpace("User");
                ulong[] weightIndices = Enumerable.Range(0, 4).Select(i => vw.HashFeature($"e{i}", nsHash)).ToArray();
                float[] values = { 1f, 2f, 3f, 4f };

                // Push everything except element 1, the way a fan-out loop excludes the candidate under test.
                VowpalWabbitExample sliced;
                using (var builder = new VowpalWabbitExampleBuilder(vw))
                {
                    using (var ns = builder.AddNamespace('U'))
                    {
                        ns.AddFeatures(weightIndices.AsSpan(0, 1), values.AsSpan(0, 1));
                        ns.AddFeatures(weightIndices.AsSpan(2), values.AsSpan(2));
                    }

                    sliced = builder.CreateExample();
                }

                VowpalWabbitExample expected;
                using (var builder = new VowpalWabbitExampleBuilder(vw))
                {
                    using (var ns = builder.AddNamespace('U'))
                    {
                        ns.AddFeature(weightIndices[0], values[0]);
                        ns.AddFeature(weightIndices[2], values[2]);
                        ns.AddFeature(weightIndices[3], values[3]);
                    }

                    expected = builder.CreateExample();
                }

                using (sliced)
                using (expected)
                {
                    Assert.AreEqual(3ul, sliced.NumberOfFeatures);
                    Assert.IsNull(sliced.Diff(vw, expected, null));
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/example builder")]
        public void TestBuilderAddFeaturesLengthMismatch()
        {
            using (VowpalWabbit vw = new VowpalWabbit("--noconstant"))
            using (var builder = new VowpalWabbitExampleBuilder(vw))
            using (var ns = builder.AddNamespace('U'))
            {
                Assert.ThrowsException<ArgumentException>(() => ns.AddFeatures(new ulong[2], new float[3]));
            }
        }
    }

    [TestClass]
    public class Test3Class : TestBase
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test3()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k train-sets/0002.dat -f models/0002.model --invariant",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002.stderr");
#endif
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test4and6()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k -d train-sets/0002.dat -f models/0002.model --invariant",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002.stderr");
#endif

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t --invariant -i models/0002.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("pred-sets", "ref", "0002b.predict"));
#else
                @"train-sets\0002.dat",
                @"pred-sets\ref\0002b.predict");
#endif
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        public void Test5()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k --initial_t 1 --adaptive --invariant -q Tf -q ff -f models/0002a.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002a.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002a.stderr");
#endif

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t --invariant -i models/0002a.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"));
#else
                @"train-sets\0002.dat");
#endif
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit/Command line through marshalling")]
        [Description("using normalized adaptive updates and a low --power_t")]
        public void Test7and8()
        {
            VWTestHelper.Learn<Data, DataListener>(
                "-k --power_t 0.45 -f models/0002c.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("train-sets", "ref", "0002c.stderr"));
#else
                @"train-sets\0002.dat",
                @"train-sets\ref\0002c.stderr");
#endif

            VWTestHelper.Predict<Data, DataListener>(
                "-k -t -i models/0002c.model",
#if NETCOREAPP3_0_OR_GREATER
                Path.Join("train-sets", "0002.dat"),
                Path.Join("pred-sets", "ref", "0002c.predict"));
#else
                @"train-sets\0002.dat",
                @"pred-sets\ref\0002c.predict");
#endif
        }
    }
}
