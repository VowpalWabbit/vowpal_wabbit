using System;
using System.Collections.Generic;

namespace VowpalWabbit.Azure.Join
{
    public class QueueDictionary<TKey, TValue>
    {
        public class Item
        {
            public TKey Key { get; set; }

            public TValue Value { get; set; }
        }

        private readonly LinkedList<Item> queue = new LinkedList<Item>();

        private readonly Dictionary<TKey, LinkedListNode<Item>> index = new Dictionary<TKey, LinkedListNode<Item>>();

        public IEnumerable<Item> DequeueIf(Predicate<TValue> pred)
        {
            while (queue.First != null && pred(queue.First.Value.Value))
            {
                var item = queue.First.Value;

                queue.RemoveFirst();
                index.Remove(item.Key);

                yield return item;
            }
        }

        public TValue Remove(TKey key)
        {
            LinkedListNode<Item> node;
            if (!index.TryGetValue(key, out node))
                return default(TValue);

            var value = node.Value.Value;

            queue.Remove(node);
            index.Remove(key);

            return value;
        }

        public void Enqueue(TKey key, TValue value)
        {
            var node = queue.AddLast(new Item { Key = key, Value = value });
            index.Add(key, node);
        }
    }
}
