using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Caching;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    public sealed class VowpalWabbitJsonReferenceResolver : IDisposable
    {
        private readonly Action<VowpalWabbitJsonSerializer> exampleComplete;
        private readonly object lockObject;
        private MemoryCache cache;
        private MemoryCache cacheRequests;
        private readonly Func<string, CacheItemPolicy> cacheItemPolicyFactory;
        private readonly Func<string, CacheItemPolicy> cacheRequestItemPolicyFactory;

        public VowpalWabbitJsonReferenceResolver(
            Action<VowpalWabbitJsonSerializer> exampleComplete,
            string cacheName = null,
            Func<string, CacheItemPolicy> cacheItemPolicyFactory = null,
            Func<string, CacheItemPolicy> cacheRequestItemPolicyFactory = null)
        {
            this.lockObject = new object();

            this.exampleComplete = exampleComplete;

            if (cacheName == null)
                cacheName = "VowpalWabbitJsonExampleCache";

            this.cacheItemPolicyFactory = cacheItemPolicyFactory == null ?
                _ => new CacheItemPolicy { SlidingExpiration = TimeSpan.FromHours(1) } :
                cacheItemPolicyFactory;

            this.cacheRequestItemPolicyFactory = cacheRequestItemPolicyFactory == null ?
                _ => new CacheItemPolicy { SlidingExpiration = TimeSpan.FromHours(1) } :
                cacheRequestItemPolicyFactory;

            this.cache = new MemoryCache(cacheName);
            this.cacheRequests = new MemoryCache(cacheName + "Requests");
        }

        internal void AddReference(string id, IVowpalWabbitMarshalAction marshalAction)
        {
            List<IncompleteReferenceRequest> requests = null;
            lock (this.lockObject)
            {
                // ignore duplicate keys - still update the sliding timer
                if (this.cache.Contains(id))
                    return;

                this.cache.Add(
                    new CacheItem(id, marshalAction),
                    this.cacheItemPolicyFactory(id));

                requests = (List<IncompleteReferenceRequest>)this.cacheRequests.Get(id);

                if (requests != null)
                {
                    foreach (var req in requests)
                        req.DontDispose = true;

                    this.cacheRequests.Remove(id);
                }
            }

            // since this can be called from another thread we need to dispatch to the serializer and let it decide
            // when to resolve the marshalling request
            if (requests != null)
            {
                foreach (var req in requests)
                    if (req.Serializer.Resolve(() => req.Marshal(marshalAction)))
                        this.exampleComplete(req.Serializer);
            }
        }

        internal void Resolve(VowpalWabbitJsonSerializer serializer, string id, Action<IVowpalWabbitMarshalAction> resolveAction)
        {
            IVowpalWabbitMarshalAction marshal;

            lock (this.lockObject)
            {
                marshal = (IVowpalWabbitMarshalAction)this.cache.Get(id);

                if (marshal == null)
                {
                    // not found, register for delayed completion
                    var requests = (List<IncompleteReferenceRequest>)this.cacheRequests.Get(id);
                    if (requests == null)
                    {
                        var policy = this.cacheRequestItemPolicyFactory(id);

                        // dispatch to original handler too
                        var removeHandler = policy.RemovedCallback;
                        if (removeHandler == null)
                            policy.RemovedCallback = this.CacheEntryRemovedCallback;
                        else
                            policy.RemovedCallback = args => { removeHandler(args); this.CacheEntryRemovedCallback(args); };

                        requests = new List<IncompleteReferenceRequest>();

                        this.cacheRequests.Add(
                            new CacheItem(id, requests),
                            policy);
                    }

                    requests.Add(
                        new IncompleteReferenceRequest
                        {
                            Serializer = serializer,
                            Marshal = resolveAction
                        });

                    serializer.IncreaseUnresolved();

                    return;
                }
            }

            // avoid extensive locking
            resolveAction(marshal);
        }

        private void CacheEntryRemovedCallback(CacheEntryRemovedArguments arguments)
        {
            lock (this.lockObject)
            {
                var requests = (List<IncompleteReferenceRequest>)arguments.CacheItem.Value;

                // dispose outstanding requests
                foreach (var request in requests)
                    if (!request.DontDispose)
                        request.Serializer.Dispose();
            }
        }

        private sealed class IncompleteReferenceRequest
        {
            internal IncompleteReferenceRequest()
            {
                this.DontDispose = false;
            }

            internal VowpalWabbitJsonSerializer Serializer { get; set; }

            internal Action<IVowpalWabbitMarshalAction> Marshal { get; set; }

            // if we return to the handler, the handler has to dispose
            internal bool DontDispose { get; set; }
        }

        public void Dispose()
        {
            if (this.cacheRequests != null)
            {
                // trigger dispose
                foreach (var key in this.cacheRequests.Select(kv => kv.Key).ToList())
                    this.cacheRequests.Remove(key);

                this.cacheRequests.Dispose();
                this.cacheRequests = null;
            }

            if (this.cache != null)
            {
                this.cache.Dispose();
                this.cache = null;
            }
        }
    }
}
