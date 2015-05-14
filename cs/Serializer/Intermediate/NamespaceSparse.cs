using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using System;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public class NamespaceSparse<TNamespaceResult, TFeatureResult> : Namespace, INamespaceSparse<TFeatureResult>, IVisitableNamespace<TNamespaceResult>
    {
        public Func<TNamespaceResult> Visit { get; set; }

        public IVisitableFeature<TFeatureResult>[] Features { get; set; }

        //override internal void ToVW(VwHandle vw, VowpalWabbitInterface.FEATURE_SPACE featureSpace)
        //{
        //    var features = new VowpalWabbitInterface.FEATURE[this.Features.Count];
        //    var pinnedsFeatures = GCHandle.Alloc(features, GCHandleType.Pinned);

        //    featureSpace.name = (byte)this.FeatureGroup;
        //    featureSpace.features = pinnedsFeatures.AddrOfPinnedObject();
        //    featureSpace.len = this.Features.Count;

        //    var namespaceHash = this.Name == null ? 0 : VowpalWabbitInterface.HashSpace(vw, this.Name);
        //    for (var i = 0; i < this.Features.Count; i++)
        //    {
        //        this.Features[i].ToVW(vw, features[i], namespaceHash);
        //    }
        //}
    }
}
