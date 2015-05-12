using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    public class VowpalWabbitNativeVisitor : IVowpalWabbitVisitor<VowpalWabbitExample, VowpalWabbitNative.FEATURE[], IEnumerable<VowpalWabbitNative.FEATURE>>
    {
        /// <summary>
        /// Performance improvement. Calculate hash once per namespace.
        /// </summary>
        private uint namespaceHash;

        public VowpalWabbitNative.FEATURE[] Visit<T>(INamespaceDense<T> namespaceDense)
        {
            throw new NotImplementedException();

            // TODO: Issues: returned cached version of Cached example
            // manage completely outside of vw
            // introduce interface!!!
        }

        public VowpalWabbitNative.FEATURE[] Visit(INamespaceSparse<IEnumerable<VowpalWabbitNative.FEATURE>> namespaceSparse)
        {
            this.namespaceHash = namespaceSparse.Name == null ? 0 : VowpalWabbitNative.HashSpace(namespaceSparse.Name);

            return namespaceSparse.Features
                .Select(f => f.Visit())
                .Where(f => f != null)
                .SelectMany(l => l)
                .ToArray();
        }

#region Numeric types

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<short> feature)
        {
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = feature.Value
            };
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<short?> feature)
        {
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = (short)feature.Value
            };
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<int> feature)
        {
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = feature.Value
            };
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<int?> feature)
        {
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = (int)feature.Value
            };
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<float> feature)
        {
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = feature.Value
            };
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<float?> feature)
        {
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = (float)feature.Value
            };
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<double> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = (float)feature.Value
            };
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit(IFeature<double?> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name, this.namespaceHash),
                x = (float)feature.Value
            };
        }

#endregion

        public IEnumerable<VowpalWabbitNative.FEATURE> VisitEnumerize<T>(IFeature<T> feature)
        {
            var strValue = Convert.ToString(feature.Value);

            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name + strValue, this.namespaceHash),
                x = 1.0f
            };
        }

        // TODO: more overloads to avoid Convert.ToDouble()

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature)
        {
            return feature.Value
                .Select(kvp => new VowpalWabbitNative.FEATURE
                {
                    weight_index = VowpalWabbitNative.HashFeature(feature.Name + Convert.ToString(kvp.Key), this.namespaceHash),
                    x = (float)Convert.ToDouble(kvp.Value)
                });
        }

        public IEnumerable<VowpalWabbitNative.FEATURE> Visit<T>(IFeature<T> feature)
        {
            string strValue;

            if (typeof(T).IsEnum)
            {
                strValue = Enum.GetName(typeof(T), feature.Value);
            }
            else
            {
                strValue = Convert.ToString(feature.Value);
            }

            yield return new VowpalWabbitNative.FEATURE
            {
                weight_index = VowpalWabbitNative.HashFeature(feature.Name + strValue, this.namespaceHash),
                x = 1.0f
            };
        }

        public VowpalWabbitExample Visit(string comment, IVisitableNamespace<VowpalWabbitNative.FEATURE[]>[] namespaces)
        {
            var features = (from n in namespaces
                            let resultFeature = n.Visit()
                            where resultFeature != null
                            select new { Namespace = n, Features = resultFeature }
                            ).ToArray();

            var featureSpaces = new VowpalWabbitNative.FEATURE_SPACE[features.Length];

            var handles = new GCHandle[features.Length];

            for (int i = 0; i < featureSpaces.Length; i++)
			{
                var featureSpace = featureSpaces[i];
                var featureNs = features[i];
			    var pinnedFeatures = GCHandle.Alloc(features[i].Features, GCHandleType.Pinned);
                handles[i] = pinnedFeatures;

                featureSpace.name = (byte)featureNs.Namespace.FeatureGroup;
                featureSpace.features = pinnedFeatures.AddrOfPinnedObject();
                featureSpace.len = featureNs.Features.Length;
			}

            return new VowpalWabbitExample(featureSpaces, handles);

            //this.namespaceOutput = new List<VowpalWabbitNative.FEATURE_SPACE>();

            //// TODO: not clear on how to caching here (and keeping track was is inserted)
            //// VisitActionDependentFeatures(string label, INamespace[],...) ?

            //visitNamespaces();

            //this.Examples.Add(new VowpalWabbitExample(this.namespaceOutput.ToArray()));



            // move to Example
            // TODO: how to handle GCHandle (need to keep in memory?)
            /// GCHandle pinnedFeatureSpace = GCHandle.Alloc(featureSpace, GCHandleType.Pinned);

            // return null; // pinnedFeatureSpace.AddrOfPinnedObject();
        }
    }
}
