using System;
using System.Collections.Generic;
using System.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Labels;
using VW.Serializer.Attributes;
using System.IO;

namespace cs_unittest
{
    [TestClass]
    public class TestPooling
    {
        [TestMethod]
        [ExpectedException(typeof(InvalidOperationException))]
        [TestCategory("Vowpal Wabbit")]
        public void ObjectPoolTestEmptyFactory()
        {
            new ObjectPool<Disposable, Disposable>().GetOrCreate();
        }

        [TestMethod]
        [ExpectedException(typeof(ObjectDisposedException))]
        [TestCategory("Vowpal Wabbit")]
        public void ObjectPoolTestDisposed1()
        {
            var objectPool = new ObjectPool<Disposable, Disposable>(ObjectFactory.Create(new Disposable(), d => d.Create()));
            objectPool.Dispose();
            objectPool.GetOrCreate();
        }

        [TestMethod]
        [ExpectedException(typeof(ObjectDisposedException))]
        [TestCategory("Vowpal Wabbit")]
        public void ObjectPoolTestDisposed2()
        {
            var objectPool = new ObjectPool<Disposable, Disposable>(ObjectFactory.Create(new Disposable(), d => d.Create()));
            objectPool.Dispose();
            objectPool.UpdateFactory(ObjectFactory.Create(new Disposable(), d => d.Create()));
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void ObjectPoolTestDangling()
        {
            var factory = new Disposable();
            var objectPool = new ObjectPool<Disposable, Disposable>(ObjectFactory.Create(factory, d => d.Create()));
            var p1 = objectPool.GetOrCreate();
            objectPool.Dispose();

            Assert.IsTrue(factory.Disposed);
            Assert.AreEqual(1, factory.Children.Count);

            Assert.IsFalse(factory.Children[0].Disposed);

            // don't throw exception if we return pool too late
            p1.Dispose();
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void ObjectPoolTestFactory()
        {
            var factory1 = new Disposable();
            var factory2 = new Disposable();
            var objectPool = new ObjectPool<Disposable, Disposable>();

            objectPool.UpdateFactory(ObjectFactory.Create(factory1, d => d.Create()));
            var p3 = objectPool.GetOrCreate();
            p3.Dispose();

            objectPool.UpdateFactory(ObjectFactory.Create(factory2, d => d.Create()));
            Assert.IsTrue(factory1.Disposed);

            var p1 = objectPool.GetOrCreate();
            var p2 = objectPool.GetOrCreate();

            p1.Dispose();
            p2.Dispose();

            p1 = objectPool.GetOrCreate();
            p1.Dispose();

            objectPool.Dispose();

            factory1.AssertChildrenDisposed();
            Assert.AreEqual(1, factory1.Children.Count);

            factory2.AssertChildrenDisposed();
            Assert.AreEqual(2, factory2.Children.Count);

            Assert.IsTrue(factory2.Disposed);
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void ThreadPoolNull()
        {
            using (var pool = new VowpalWabbitThreadedPrediction())
            {
                using (var vw = pool.GetOrCreate())
                {
                    Assert.IsNull(vw.Value);
                }

                pool.UpdateModel(new VowpalWabbitModel(string.Empty));

                using (var vw = pool.GetOrCreate())
                {
                    Assert.IsNotNull(vw.Value);
                }
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void ObjectPoolTestConcurrency()
        {
            var factories = new List<Disposable> { new Disposable() };
            var objectPool = new ObjectPool<Disposable, Disposable>(ObjectFactory.Create(factories[0], d => d.Create()));

            var t1 = new Thread(() =>
            {
                for (int i = 0; i < 500; i++)
                {
                    var p = objectPool.GetOrCreate();
                    Thread.Sleep(5);
                    p.Dispose();
                }
            });

            var t2 = new Thread(() =>
            {
                for (int i = 0; i < 500; i++)
                {
                    var p = objectPool.GetOrCreate();
                    Thread.Sleep(7);
                    p.Dispose();
                }
            });

            var t3 = new Thread(() =>
            {
                for (int i = 0; i < 20; i++)
                {
                    var f = new Disposable();
                    objectPool.UpdateFactory(ObjectFactory.Create(f, d => d.Create()));
                    Thread.Sleep(20);
                }
            });

            t1.Start();
            t2.Start();
            t3.Start();

            t1.Join();
            t2.Join();
            t3.Join();

            objectPool.Dispose();

            foreach (var f in factories)
            {
                Assert.IsTrue(f.Disposed);

                f.AssertChildrenDisposed();
            }
        }

        public class SimpleData
        {
            [Feature]
            public float Value { get; set; }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestSaveLoadSkip()
        {
            using (var vw = new VowpalWabbit<SimpleData>("--binary -f saveload.model"))
            {
                for (int i = 0; i < 100; i++)
                {
                    vw.Learn(new SimpleData { Value = 1 }, new SimpleLabel { Label = 1 });
                    vw.Learn(new SimpleData { Value = -1 }, new SimpleLabel { Label = -1 });
                }

                Assert.AreEqual(1, vw.Predict(new SimpleData { Value = 1 }, VowpalWabbitPredictionType.Scalar));
                Assert.AreEqual(-1, vw.Predict(new SimpleData { Value = -1 }, VowpalWabbitPredictionType.Scalar));
            }

            using (var model = new VowpalWabbitModel(new VowpalWabbitSettings { Arguments = "--binary", ModelStream = File.Open("saveload.model", FileMode.Open) }))
            using (var pool = new VowpalWabbitThreadedPrediction<SimpleData>(new VowpalWabbitSettings { Model = model }))
            {
                using (var vw = pool.GetOrCreate())
                {
                    Assert.AreEqual(-1, vw.Value.Predict(new SimpleData { Value = -1 }, VowpalWabbitPredictionType.Scalar));
                }
            }
        }

        public class Disposable : IDisposable
        {
            public Disposable()
            {
                this.Children = new List<Disposable>();
            }

            public void AssertChildrenDisposed()
            {
                foreach (var item in this.Children)
                {
                    Assert.IsTrue(item.Disposed);
                }
            }

            public List<Disposable> Children { get; set; }

            public bool Disposed { get; set; }

            public void Dispose()
            {
                this.Disposed = true;
            }

            public Disposable Create()
            {
                var d = new Disposable();
                this.Children.Add(d);
                return d;
            }
        }
    }
}
