using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    public class VowpalWabbitNativeVisitor : IVowpalWabbitVisitor<VowpalWabbitExample, VowpalWabbitNative.FEATURE_SPACE, VowpalWabbitNative.FEATURE>
    {
        public void Visit<T>(INamespaceDense<T> namespaceDense)
        {
            throw new NotImplementedException();
        }

        public IList<VowpalWabbitExample> Examples { get; private set; }

        public VowpalWabbitNative.FEATURE_SPACE Visit(INamespaceSparse namespaceSparse, Action visitFeatures)
        {
            var features = new VowpalWabbitNative.FEATURE[this.Features.Count];
            var pinnedsFeatures = GCHandle.Alloc(features, GCHandleType.Pinned);

            featureSpace.name = (byte)this.FeatureGroup;
            featureSpace.features = pinnedsFeatures.AddrOfPinnedObject();
            featureSpace.len = this.Features.Count;

            var namespaceHash = this.Name == null ? 0 : VowpalWabbitNative.HashSpace(vw, this.Name);
            for (var i = 0; i < this.Features.Count; i++)
            {
                this.Features[i].ToVW(vw, features[i], namespaceHash);
            }

        }

        public void Visit<T>(IFeature<T> feature)
        {
            throw new NotImplementedException();
        }

        public void Initialize()
        {
            this.Examples = new List<VowpalWabbitExample>();
        }

        public VowpalWabbitExample Visit(string comment, INamespace[] namespaces, Action visitNamespaces)
        {
            this.namespaceOutput = new List<VowpalWabbitNative.FEATURE_SPACE>();

            // TODO: not clear on how to caching here (and keeping track was is inserted)
            // VisitActionDependentFeatures(string label, INamespace[],...) ?

            visitNamespaces();

            this.Examples.Add(new VowpalWabbitExample(this.namespaceOutput.ToArray()));



            // move to Example
            // TODO: how to handle GCHandle (need to keep in memory?)
            /// GCHandle pinnedFeatureSpace = GCHandle.Alloc(featureSpace, GCHandleType.Pinned);

            // return null; // pinnedFeatureSpace.AddrOfPinnedObject();
        }
    }
}
