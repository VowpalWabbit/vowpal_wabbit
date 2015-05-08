using VowpalWabbit.Serializer.Interfaces;

namespace VowpalWabbit.Serializer.Intermediate
{
    public class NamespaceSparse : Namespace, INamespaceSparse
    {
        public IFeature[] Features { get; set; }

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
