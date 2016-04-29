using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW
{
    public interface IBag<T>
    {
        void Add(T item);

        T Remove();

        IEnumerable<T> RemoveAll();
    }

    public static class Bag
    {
        public static IBag<T> Synchronized<T>(IBag<T> bag)
        {
            return new ThreadSafeBagImpl<T>(bag);
        }

        private sealed class ThreadSafeBagImpl<T> : IBag<T>
        {
            private readonly object lockObj;
            private readonly IBag<T> bag;

            public ThreadSafeBagImpl(IBag<T> bag)
            {
                if (bag == null)
                    throw new ArgumentNullException("bag");

                this.bag = bag;
                this.lockObj = new object();
            }

            public void Add(T item)
            {
                lock (this.lockObj)
                {
                    this.bag.Add(item);
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
        }
    }

    public sealed class Bag<T> : IBag<T>
    {
        private readonly Stack<T> stack;

        public Bag()
        {
            this.stack = new Stack<T>();
        }

        public void Add(T item)
        {
            this.stack.Push(item);
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
    }
}
