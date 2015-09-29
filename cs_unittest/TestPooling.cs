using System;
using System.Collections.Generic;
using System.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using VW;
using VW.Interfaces;

namespace cs_unittest
{
    [TestClass]
    public class TestPooling
    {
        [TestMethod]
        [ExpectedException(typeof(InvalidOperationException))]
        public void ObjectPoolTestEmptyFactory()
        {
            new ObjectPool<Disposable, Disposable>().GetOrCreate();
        }

        [TestMethod]
        [ExpectedException(typeof(ObjectDisposedException))]
        public void ObjectPoolTestDisposed1()
        {
            var objectPool = new ObjectPool<Disposable, Disposable>(ObjectFactory.Create(new Disposable(), d => d.Create()));
            objectPool.Dispose();
            objectPool.GetOrCreate();
        }

        [TestMethod]
        [ExpectedException(typeof(ObjectDisposedException))]
        public void ObjectPoolTestDisposed2()
        {
            var objectPool = new ObjectPool<Disposable, Disposable>(ObjectFactory.Create(new Disposable(), d => d.Create()));
            objectPool.Dispose();
            objectPool.UpdateFactory(ObjectFactory.Create(new Disposable(), d => d.Create()));
        }

        [TestMethod]
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
