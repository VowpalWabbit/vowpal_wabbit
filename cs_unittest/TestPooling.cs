using VW;
using VW.Interfaces;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public class TestPooling
    {
        [TestMethod]
        [ExpectedException(typeof(InvalidOperationException))]
        public void ObjectPoolTestEmptyFactory()
        {
            new ObjectPool<Disposable>().Get();
        }

        [TestMethod]
        [ExpectedException(typeof(ObjectDisposedException))]
        public void ObjectPoolTestDisposed1()
        {
            var objectPool = new ObjectPool<Disposable>(new Disposable());
            objectPool.Dispose();
            objectPool.Get();
        }

        [TestMethod]
        [ExpectedException(typeof(ObjectDisposedException))]
        public void ObjectPoolTestDisposed2()
        {
            var objectPool = new ObjectPool<Disposable>(new Disposable());
            objectPool.Dispose();
            objectPool.UpdateFactory(new Disposable());
        }

        [TestMethod]
        public void ObjectPoolTestDangling()
        {
            var factory = new Disposable();
            var objectPool = new ObjectPool<Disposable>(factory);
            var p1 = objectPool.Get();
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
            var objectPool = new ObjectPool<Disposable>();

            objectPool.UpdateFactory(factory1);
            var p3 = objectPool.Get();
            p3.Dispose();

            objectPool.UpdateFactory(factory2);
            Assert.IsTrue(factory1.Disposed);

            var p1 = objectPool.Get();
            var p2 = objectPool.Get();

            p1.Dispose();
            p2.Dispose();

            p1 = objectPool.Get();
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
            var objectPool = new ObjectPool<Disposable>(factories[0]);

            var t1 = new Thread(() =>
            {
                for (int i = 0; i < 500; i++)
                {
                    var p = objectPool.Get();
                    Thread.Sleep(5);
                    p.Dispose();
                }
            });

            var t2 = new Thread(() =>
            {
                for (int i = 0; i < 500; i++)
                {
                    var p = objectPool.Get();
                    Thread.Sleep(7);
                    p.Dispose();
                }
            });

            var t3 = new Thread(() =>
            {
                for (int i = 0; i < 20; i++)
                {
                    var f = new Disposable();
                    objectPool.UpdateFactory(f);
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


        public class Disposable : IDisposable, IObjectFactory<Disposable>
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
