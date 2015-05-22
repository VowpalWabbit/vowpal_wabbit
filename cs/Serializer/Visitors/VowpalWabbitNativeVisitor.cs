// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitInterfaceVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    /// <summary>
    /// Front-end to serialize data into Vowpal Wabbit native C++ structures.
    /// </summary>
    public class VowpalWabbitInterfaceVisitor : IVowpalWabbitVisitor<VowpalWabbitExample, FEATURE[], IEnumerable<FEATURE>>
    {
        private readonly VowpalWabbit vw;

        /// <summary>
        /// Performance improvement. Calculate hash once per namespace.
        /// </summary>
        private uint namespaceHash;

        public VowpalWabbitInterfaceVisitor(VowpalWabbit vw)
        {
            this.vw = vw;
        }

        public FEATURE[] Visit<T>(INamespaceDense<T> namespaceDense)
        {
            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(namespaceDense.FeatureGroup.ToString()) :
                this.vw.HashSpace(namespaceDense.FeatureGroup + namespaceDense.Name);

            return namespaceDense.DenseFeature.Value
                .Select((v, i) => new FEATURE
                {
                    weight_index = (uint) (this.namespaceHash + i),
                    x = (float) Convert.ToDouble(v)
                })
                .ToArray();
        }

        public FEATURE[] Visit(INamespaceSparse<IEnumerable<FEATURE>> namespaceSparse)
        {
            this.namespaceHash = namespaceSparse.Name == null ? 
                this.vw.HashSpace(namespaceSparse.FeatureGroup.ToString()) :
                this.vw.HashSpace(namespaceSparse.FeatureGroup + namespaceSparse.Name);

            return namespaceSparse.Features
                .Select(f => f.Visit())
                .Where(f => f != null)
                .SelectMany(l => l)
                .ToArray();
        }

        public IEnumerable<FEATURE> Visit(IFeature<short> feature)
        {
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = feature.Value
            };
        }

        public IEnumerable<FEATURE> Visit(IFeature<short?> feature)
        {
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = (short)feature.Value
            };
        }

        public IEnumerable<FEATURE> Visit(IFeature<int> feature)
        {
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = feature.Value
            };
        }

        public IEnumerable<FEATURE> Visit(IFeature<int?> feature)
        {
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = (int)feature.Value
            };
        }

        public IEnumerable<FEATURE> Visit(IFeature<float> feature)
        {
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = feature.Value
            };
        }

        public IEnumerable<FEATURE> Visit(IFeature<float?> feature)
        {
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = (float)feature.Value
            };
        }

        public IEnumerable<FEATURE> Visit(IFeature<double> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = (float)feature.Value
            };
        }

        public IEnumerable<FEATURE> Visit(IFeature<double?> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name, this.namespaceHash),
                x = (float)feature.Value
            };
        }

        public IEnumerable<FEATURE> VisitEnumerize<T>(IFeature<T> feature)
        {
            var strValue = Convert.ToString(feature.Value);

            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name + strValue, this.namespaceHash),
                x = 1.0f
            };
        }

        public IEnumerable<FEATURE> Visit<TValue>(IFeature<IDictionary<UInt16, TValue>> feature)
        {
            return feature.Value
                .Select(kvp => new FEATURE
                {
                    weight_index = this.namespaceHash + kvp.Key,
                    x = (float)Convert.ToDouble(kvp.Value)
                });
        }

        public IEnumerable<FEATURE> Visit<TValue>(IFeature<IDictionary<UInt32, TValue>> feature)
        {
            return feature.Value
                .Select(kvp => new FEATURE
                {
                    weight_index = this.namespaceHash + kvp.Key,
                    x = (float)Convert.ToDouble(kvp.Value)
                });
        }

        public IEnumerable<FEATURE> Visit<TValue>(IFeature<IDictionary<Int16, TValue>> feature)
        {
            return feature.Value
                .Select(kvp => new FEATURE
                {
                    weight_index = (uint)(this.namespaceHash + kvp.Key),
                    x = (float)Convert.ToDouble(kvp.Value)
                });
        }

        public IEnumerable<FEATURE> Visit<TValue>(IFeature<IDictionary<Int32, TValue>> feature)
        {
            return feature.Value
                .Select(kvp => new FEATURE
                {
                    weight_index = (uint)(this.namespaceHash + kvp.Key),
                    x = (float)Convert.ToDouble(kvp.Value)
                });
        }

        public IEnumerable<FEATURE> Visit(IFeature<IDictionary<Int32, float>> feature)
        {
            return feature.Value
                .Select(kvp => new FEATURE
                {
                    weight_index = (uint)(this.namespaceHash + kvp.Key),
                    x = kvp.Value
                });
        }

        public IEnumerable<FEATURE> Visit<TKey, TValue>(IFeature<IEnumerable<KeyValuePair<TKey, TValue>>> feature)
        {
            return feature.Value
                .Select(kvp => new FEATURE
                {
                    weight_index = this.vw.HashFeature(Convert.ToString(kvp.Key), this.namespaceHash),
                    x = (float)Convert.ToDouble(kvp.Value)
                });
        }


        public IEnumerable<FEATURE> Visit(IFeature<IDictionary> feature)
        {
            // lhs: int, hash(string), hash(long), hash(*) -> uint
            // rhs: int, short, long, float, bool -> float

            var features = new FEATURE[feature.Value.Count];
            var i = 0;
            foreach (DictionaryEntry item in feature.Value)
            {
                features[i].weight_index = this.vw.HashFeature(Convert.ToString(item.Key), this.namespaceHash);
                features[i].x = (float) Convert.ToDouble(item.Value);
            }
        
            return features;
        }

        public IEnumerable<FEATURE> Visit(IFeature<IEnumerable<string>> feature)
        {
            return feature.Value
                .Select(value => new FEATURE
                {
                    weight_index = this.vw.HashFeature(value, this.namespaceHash),
                    x = 1f
                });
        }

        public IEnumerable<FEATURE> Visit<T>(IFeature<T> feature)
        {
            var  strValue = typeof(T).IsEnum ? 
                Enum.GetName(typeof(T), feature.Value) : Convert.ToString(feature.Value);

            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name + strValue, this.namespaceHash),
                x = 1.0f
            };
        }

        public VowpalWabbitExample Visit(string label, IVisitableNamespace<FEATURE[]>[] namespaces)
        {
            var featureSpaces = (from n in namespaces
                            let resultFeature = n.Visit()
                            where resultFeature != null
                            select new FeatureSpace 
                            { 
                                Name = (byte)(n.FeatureGroup ?? 0), 
                                Features = resultFeature 
                            }).ToArray();

            // move data into VW
            return this.vw.ImportExample(label, featureSpaces);
        }
    }
}
