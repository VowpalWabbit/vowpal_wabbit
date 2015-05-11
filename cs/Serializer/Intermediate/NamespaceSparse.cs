using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using System;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    public class NamespaceSparse : Namespace, INamespaceSparse, IVisitableNamespace<TResult>
    {
        private Func<INamespaceDense<T>> dispatch;

        public NamespaceSparse(Func<INamespaceDense<T>> dispatch)
        {
            this.dispatch = dispatch;
        }

        public IFeature[] Features { get; set; }

        public TResult Visit()
        {
            return this.dispatch(this);
        }

        //override internal void ToVW(VwHandle vw, VowpalWabbitNative.FEATURE_SPACE featureSpace)
        //{
        //    var features = new VowpalWabbitNative.FEATURE[this.Features.Count];
        //    var pinnedsFeatures = GCHandle.Alloc(features, GCHandleType.Pinned);

        //    featureSpace.name = (byte)this.FeatureGroup;
        //    featureSpace.features = pinnedsFeatures.AddrOfPinnedObject();
        //    featureSpace.len = this.Features.Count;

        //    var namespaceHash = this.Name == null ? 0 : VowpalWabbitNative.HashSpace(vw, this.Name);
        //    for (var i = 0; i < this.Features.Count; i++)
        //    {
        //        this.Features[i].ToVW(vw, features[i], namespaceHash);
        //    }
        //}
    }
}
