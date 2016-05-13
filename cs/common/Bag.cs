using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
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

        public static IBag<T> CreateLockFree<T>()
        {
            return new LockFreeBagImpl<T>();
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

        private sealed class LockFreeBagImpl<T> : IBag<T>
        {
            private ConcurrentQueue<T> queue;

            internal LockFreeBagImpl()
            {
                this.queue = new ConcurrentQueue<T>();
            }

            public bool TryAdd(T item)
            {
                this.queue.Enqueue(item);
                return true;
            }

            public T Remove()
            {
                T result;
                if (this.queue.TryDequeue(out result))
                    return result;

                return default(T);
            }

            public IEnumerable<T> RemoveAll()
            {
                // TODO: violates the lock constraint. though this is just used at disposable time
                return this.queue;
            }

            public int Count
            {
                get { return this.queue.Count; }
            }
        }
    }
}
