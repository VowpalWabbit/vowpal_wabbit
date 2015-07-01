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
using VW.Serializer.Interfaces;

namespace VW.Serializer.Visitors
{
        /// 
    /// <summary>
    /// Front-end to serialize data into Vowpal Wabbit native C++ structures.
    /// </summary>
    public sealed class VowpalWabbitInterfaceVisitor : IVowpalWabbitVisitor<VowpalWabbitExample>
    {
        /// <summary>
        /// The Vowpal Wabbit instance all examples are associated with.
        /// </summary>
        private readonly VowpalWabbit vw;

        /// <summary>
        /// Performance improvement. Calculated hash once per namespace.
        /// </summary>
        private uint namespaceHash;

        private char featureGroup;

        private VowpalWabbitExampleBuilder builder;

        private VowpalWabbitNamespaceBuilder namespaceBuilder;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitInterfaceVisitor"/> instance.
        /// </summary>
        /// <param name="vw">The associated vowpal wabbit instance.</param>
        public VowpalWabbitInterfaceVisitor(VowpalWabbit vw)
        {
            this.vw = vw;
        }

        /// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <typeparam name="T">The feature type.</typeparam>
        /// <param name="namespaceDense">The dense namespace.</param>
        public void Visit<T>(INamespaceDense<T> namespaceDense)
        {
            this.featureGroup = namespaceDense.FeatureGroup ?? '\0';

            this.namespaceHash = namespaceDense.Name == null ? 
                this.vw.HashSpace(this.featureGroup.ToString()) :
                this.vw.HashSpace(this.featureGroup + namespaceDense.Name);

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);

            var i = 0;
            foreach (var v in namespaceDense.DenseFeature.Value)
            {
                this.namespaceBuilder.AddFeature(
                    (uint) (this.namespaceHash + i),
                    (float) Convert.ToDouble(v));
                i++;
            }
        }

        /// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="namespaceSparse">The sparse namespace.</param>
        public void Visit(INamespaceSparse namespaceSparse)
        {
            // compute shared namespace hash
            this.namespaceHash = namespaceSparse.Name == null ? 
                this.vw.HashSpace(namespaceSparse.FeatureGroup.ToString()) :
                this.vw.HashSpace(namespaceSparse.FeatureGroup + namespaceSparse.Name);

            this.featureGroup = namespaceSparse.FeatureGroup ?? '\0';

            this.namespaceBuilder = this.builder.AddNamespace(this.featureGroup);

            // Visit each feature
            foreach (var element in namespaceSparse.Features)
            {
                element.Visit();
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<short> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<short?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (short)feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<int> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<int?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (int)feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<float> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<float?> feature)
        {
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<double> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<double?> feature)
        {
#if DEBUG
            if (feature.Value > float.MaxValue || feature.Value < float.MinValue)
            {
                Trace.TraceWarning("Precision lost for feature value: " + feature.Value);
            }
#endif
            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name, this.namespaceHash), (float)feature.Value);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void VisitEnumerize<T>(IFeature<T> feature)
        {
            var strValue = Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<UInt16, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash + kvp.Key, (float)Convert.ToDouble(kvp.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<UInt32, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature(this.namespaceHash + kvp.Key, (float)Convert.ToDouble(kvp.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<Int16, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TValue>(IFeature<IDictionary<Int32, TValue>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), (float)Convert.ToDouble(kvp.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary<Int32, float>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature((uint)(this.namespaceHash + kvp.Key), kvp.Value);
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<TKey, TValue>(IFeature<IEnumerable<KeyValuePair<TKey, TValue>>> feature)
        {
            foreach (var kvp in feature.Value)
            {
                this.namespaceBuilder.AddFeature(
                        this.vw.HashFeature(Convert.ToString(kvp.Key), this.namespaceHash),
                        (float)Convert.ToDouble(kvp.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IDictionary> feature)
        {
            foreach (DictionaryEntry item in feature.Value)
            {
                this.namespaceBuilder.AddFeature(
                    this.vw.HashFeature(Convert.ToString(item.Key), this.namespaceHash),
                    (float) Convert.ToDouble(item.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit(IFeature<IEnumerable<string>> feature)
        {
            foreach (var value in feature.Value)
            {
                this.namespaceBuilder.AddFeature(this.vw.HashFeature(value, this.namespaceHash), 1f);
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void Visit<T>(IFeature<T> feature)
        {
            var strValue = typeof(T).IsEnum ? 
                Enum.GetName(typeof(T), feature.Value) : Convert.ToString(feature.Value);

            this.namespaceBuilder.AddFeature(this.vw.HashFeature(feature.Name + strValue, this.namespaceHash), 1f);
        }

        /// <summary>
        /// Transfers namespace data to native space.
        /// </summary>
        /// <param name="label">The label.</param>
        /// <param name="namespaces">The namespaces.</param>
        /// <returns>The populated vowpal wabbit example.</returns>
        public VowpalWabbitExample Visit(string label, IVisitableNamespace[] namespaces)
        {
            using (this.builder = new VowpalWabbitExampleBuilder(this.vw))
            {
                this.builder.Label = label;

                foreach (var n in namespaces)
                {
                    n.Visit();
                }

                return this.builder.CreateExample();
            }
        }
    }
}
