using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW
{
    public interface IBag<T>
    {
        bool TryAdd(T item);

        T Remove();

        IEnumerable<T> RemoveAll();

        int Count { get; }
    }

    public static class Bag
    {
        public static IBag<T> Create<T>(int max = int.MaxValue)
        {
            return max == int.MaxValue ?
                (IBag<T>)new BagImpl<T>() : new BoundedBagImpl<T>(max);
        }

        public static IBag<T> Synchronized<T>(IBag<T> bag)
        {
            return new ThreadSafeBagImpl<T>(bag);
        }

        private abstract class BaseBagImpl<T>
        {
            protected readonly Stack<T> stack;

            internal BaseBagImpl()
            {
                this.stack = new Stack<T>();
            }

            public T Remove()
            {
                return this.stack.Count == 0 ? default(T) : this.stack.Pop();
            }

            public IEnumerable<T> RemoveAll()
            {
                var ret = this.stack.ToArray();
                this.stack.Clear();

                return ret;
            }

            public int Count
            {
                get { return this.stack.Count; }
            }
        }

        private sealed class BagImpl<T> : BaseBagImpl<T>, IBag<T>
        {
            public bool TryAdd(T item)
            {
                this.stack.Push(item);
                return true;
            }
        }

        private sealed class BoundedBagImpl<T> : BaseBagImpl<T>, IBag<T>
        {
            private readonly int max;

            internal BoundedBagImpl(int max)
            {
                this.max = max;
            }

            public bool TryAdd(T item)
            {
                if (this.stack.Count >= this.max)
                    return false;

                this.stack.Push(item);
                return true;
            }
        }

        private sealed class ThreadSafeBagImpl<T> : IBag<T>
        {
            private readonly object lockObj;
            private readonly IBag<T> bag;

            internal ThreadSafeBagImpl(IBag<T> bag)
            {
                if (bag == null)
                    throw new ArgumentNullException("bag");

                this.bag = bag;
                this.lockObj = new object();
            }

            public bool TryAdd(T item)
            {
                lock (this.lockObj)
                {
                    return this.bag.TryAdd(item);
                }
            }

            public T Remove()
            {
                lock (this.lockObj)
                {
                    return this.bag.Remove();
                }
            }

            public IEnumerable<T> RemoveAll()
            {
                lock (this.lockObj)
                {
                    return this.bag.RemoveAll();
                }
            }

            public int Count
            {
                get
                {
                    lock (this.lockObj)
                    {
                        return this.bag.Count;
                    }
                }
            }
        }
    }
}
