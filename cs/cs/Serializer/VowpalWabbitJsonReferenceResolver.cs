using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    public sealed class VowpalWabbitJsonReferenceResolver : IDisposable
    {
        private readonly Action<VowpalWabbitJsonSerializer> exampleComplete;
        private readonly object lockObject;
        // TODO: MemoryCollection using timeouts and maximum size
        private readonly Dictionary<string, IVowpalWabbitMarshalAction> dictionary;
        private Dictionary<string, List<IncompleteReferenceRequest>> incompleteReferenceRequests;

        public VowpalWabbitJsonReferenceResolver(Action<VowpalWabbitJsonSerializer> exampleComplete)
        {
            this.exampleComplete = exampleComplete;

            this.dictionary = new Dictionary<string, IVowpalWabbitMarshalAction>();
            this.incompleteReferenceRequests = new Dictionary<string, List<IncompleteReferenceRequest>>();
            this.lockObject = new object();
        }

        internal void Resolve(VowpalWabbitJsonSerializer serializer, string id, Action<IVowpalWabbitMarshalAction> resolveAction)
        {
            IVowpalWabbitMarshalAction marshal;

            lock (this.lockObject)
            {
                if (!this.dictionary.TryGetValue(id, out marshal))
                {
                    // not found, register for delayed completion
                    List<IncompleteReferenceRequest> requests;
                    if (!this.incompleteReferenceRequests.TryGetValue(id, out requests))
                    {
                        requests = new List<IncompleteReferenceRequest>();
                        this.incompleteReferenceRequests.Add(id, requests);
                    }

                    serializer.IncreaseUnresolved();

                    requests.Add(
                        new IncompleteReferenceRequest
                        {
                            Serializer = serializer,
                            Marshal = resolveAction
                        });

                    return;
                }
            }

            // avoid extensive locking
            resolveAction(marshal);
        }

        internal void AddReference(string id, IVowpalWabbitMarshalAction marshalAction)
        {
            List<IncompleteReferenceRequest> requests = null;
            lock (this.lockObject)
            {
                // ignore duplicate keys
                if (this.dictionary.ContainsKey(id))
                    return;

                this.dictionary.Add(id, marshalAction);

                if (this.incompleteReferenceRequests.TryGetValue(id, out requests))
                    this.incompleteReferenceRequests.Remove(id);
            }

            // since this can be called from another thread we need to dispatch to the serializer and let it decide
            // when to resolve the marshalling request
            if (requests != null)
                foreach (var req in requests)
                    if (req.Serializer.Resolve(() => req.Marshal(marshalAction)))
                        this.exampleComplete(req.Serializer);
        }

        private sealed class IncompleteReferenceRequest
        {
            internal VowpalWabbitJsonSerializer Serializer { get; set; }

            internal Action<IVowpalWabbitMarshalAction> Marshal { get; set; }
        }

        public void Dispose()
        {
            if (this.incompleteReferenceRequests != null)
            {
                foreach (var requests in this.incompleteReferenceRequests.Values)
                    foreach (var request in requests)
                        request.Serializer.Dispose();

                this.incompleteReferenceRequests = null;
            }
        }
    }
}
