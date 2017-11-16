using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace VW
{
    /// <summary>
    /// Interface to model simple bag.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface IBag<T>
    {
        /// <summary>
        /// Try add <paramref name="item"/> to this bag.
        /// </summary>
        /// <param name="item">The item to add.</param>
        /// <returns>True if succesful, false otherwise.</returns>
        bool TryAdd(T item);

        /// <summary>
        /// Remove and return one item from this bag.
        /// </summary>
        /// <returns>The item removed from the bag or default(T) if there is no item available.</returns>
        T Remove();

        /// <summary>
        /// Remove and return all items from this bag.
        /// </summary>
        /// <returns>The items removed from the gag.</returns>
        IEnumerable<T> RemoveAll();

        /// <summary>
        /// The number of items this bag contains.
        /// </summary>
        int Count { get; }
    }

    /// <summary>
    /// Factory class for various bag implementations.
    /// </summary>
    public static class Bag
    {
        /// <summary>
        /// Creates a simple bound or unbound, not thread-safe bag object.
        /// </summary>
        /// <typeparam name="T">The type of the items.</typeparam>
        /// <param name="max">The maximum number of items this bag should hold.</param>
        /// <returns>A new bag instance.</returns>
        public static IBag<T> Create<T>(int max = int.MaxValue)
        {
            return max == int.MaxValue ?
                (IBag<T>)new BagImpl<T>() : new BoundedBagImpl<T>(max);
        }

        /// <summary>
        /// Creates an unbound thread-safe, lock free bag.
        /// </summary>
        /// <typeparam name="T">The type of the items.</typeparam>
        /// <returns>A new bag instance.</returns>
        public static IBag<T> CreateLockFree<T>(int max = int.MaxValue)
        {
            return new LockFreeBagImpl<T>(max);
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

        /// <summary>
        /// This is a good read on performance: http://msdn.microsoft.com/en-us/concurrency/ee851578.aspx
        /// For streaming training we are seeking good performance for a single producer and multiple consumers.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        private sealed class LockFreeBagImpl<T> : IBag<T>
        {
            private readonly int max;
            private readonly ConcurrentQueue<T> queue;
            private int count;

            internal LockFreeBagImpl(int max)
            {
                this.queue = new ConcurrentQueue<T>();
                this.max = max;
            }

            public bool TryAdd(T item)
            {
                if (this.count < this.max)
                {
                    this.queue.Enqueue(item);
                    Interlocked.Increment(ref this.count);
                    return true;
                }

                return false;
            }

            public T Remove()
            {
                T result;
                if (this.queue.TryDequeue(out result))
                {
                    Interlocked.Decrement(ref this.count);
                    return result;
                }

                return default(T);
            }

            public IEnumerable<T> RemoveAll()
            {
                // TODO: violates the lock constraint. though this is just used at disposable time
                return this.queue;
            }

            public int Count
            {
                get { return this.count; }
            }
        }
    }
}
