using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    public class VowpalWabbitInterfaceVisitor : IVowpalWabbitVisitor<VowpalWabbitExample, FEATURE[], IEnumerable<FEATURE>>
    {
        private readonly VowpalWabbit vw;

        public VowpalWabbitInterfaceVisitor(VowpalWabbit vw)
        {
            this.vw = vw;
        }

        /// <summary>
        /// Performance improvement. Calculate hash once per namespace.
        /// </summary>
        private uint namespaceHash;

        public FEATURE[] Visit<T>(INamespaceDense<T> namespaceDense)
        {
            throw new NotImplementedException();

            // TODO: Issues: returned cached version of Cached example
            // manage completely outside of vw
            // introduce interface!!!
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

#region Numeric types

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

#endregion

        public IEnumerable<FEATURE> VisitEnumerize<T>(IFeature<T> feature)
        {
            var strValue = Convert.ToString(feature.Value);

            yield return new FEATURE
            {
                weight_index = this.vw.HashFeature(feature.Name + strValue, this.namespaceHash),
                x = 1.0f
            };
        }

        #region Dictionary

        // TODO: more overloads to avoid Convert.ToDouble()

        /*        
public void Visit<TValue>(IFeature<IDictionary<Int16, TValue>> feature)
{
    this.Visit(feature, key => (UInt32)key);
}

public void Visit<TValue>(IFeature<IDictionary<Int32, TValue>> feature)
{
    this.Visit(feature, key => (UInt32)key);
}

// TODO: not clear when to hash... should we check for negative values?
public void Visit<TValue>(IFeature<IDictionary<Int64, TValue>> feature)
{
    this.Visit(feature, key => (UInt32)key);
}

public void Visit<TValue>(IFeature<IDictionary<UInt16, TValue>> feature)
{
    this.Visit(feature, key => key);
}
public void Visit<TValue>(IFeature<IDictionary<UInt32, TValue>> feature)
{
    this.Visit(feature, key => key);
}
*/
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

        public IEnumerable<FEATURE> Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature)
        {
            // lhs: int, hash(string), hash(long), hash(*) -> uint
            // rhs: int, short, long, float, bool -> float

            return feature.Value
                .Select(kvp => new FEATURE
                {
                    weight_index = this.vw.HashFeature(feature.Name + Convert.ToString(kvp.Key), this.namespaceHash),
                    x = (float)Convert.ToDouble(kvp.Value)
                });
        }

        #endregion

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

        public VowpalWabbitExample Visit(IVisitableNamespace<FEATURE[]>[] namespaces)
        {
            var featureSpaces = (from n in namespaces
                            let resultFeature = n.Visit()
                            where resultFeature != null
                            select new FeatureSpace 
                            { 
                                Name = (byte)(n.FeatureGroup ?? 0), 
                                Features = resultFeature 
                            }).ToArray();

            return this.vw.ImportExample(featureSpaces);
        }
    }
}
