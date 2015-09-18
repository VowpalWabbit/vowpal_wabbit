// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitStringVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Globalization;
using System.Linq;
using System.Text;
using VW.Interfaces;
using VW.Serializer.Inspectors;
using VW.Serializer.Interfaces;

#pragma warning disable 1591

namespace VW.Serializer.Visitors
{
    /// <summary>
    /// Front-end to serialize data into Vowpal Wabbit string format.
    /// </summary>
    public struct VowpalWabbitStringVisitor 
    {
        private StringBuilder builder;

        public void Visit<T>(INamespaceDense<T> namespaceDense)
        {
            Contract.Requires(namespaceDense != null);

            this.builder.AppendFormat(
                CultureInfo.InvariantCulture,
                " |{0}{1}",
                namespaceDense.FeatureGroup,
                namespaceDense.Name);

            var i = 0;

            // support anchor feature
            if (namespaceDense.DenseFeature.AddAnchor)
            {
                this.builder.Append(" 0:1");
                i++;
            }

            foreach (var value in namespaceDense.DenseFeature.Value)
            {
                this.builder.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}:{1}",
                    i,
                    value);

                i++;
            }
        }

        public void Visit(IFeature<string> feature)
        {
            Contract.Requires(feature != null);
            
            // TODO: either replace spaces, throw a warning or ignore, quoting???
            this.builder.Append(' ').Append(feature.Value);
        }

        public void Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature)
        {
            Contract.Requires(feature != null);

            this.Visit(feature, key => Convert.ToString(key));
        }

        private void Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature, Func<TKey, string> keyMapper)
        {
            Contract.Requires(feature != null);
            Contract.Requires(keyMapper != null);

            var first = true;
            foreach (var kvp in feature.Value)
            {
                if (!first)
                {
                    this.builder.Append(" ");
                    first = false;
                }
                this.builder.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}:{1}",
                    keyMapper(kvp.Key),
                    kvp.Value);
            }
        }

        public void Visit<TValue>(IFeature<IEnumerable<TValue>> feature)
        {
            Contract.Requires(feature != null);

            var i = 0;
            foreach (var value in feature.Value)
            {
                if (i > 0)
                {
                    this.builder.Append(" ");
                }
                this.builder.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}:{1}",
                    i,
                    value);

                i++;
            }
        }

        public void VisitEnumerize<T>(IFeature<T> feature)
        {
            Contract.Requires(feature != null);

            this.builder.AppendFormat(
                CultureInfo.InvariantCulture,
                " {0}_{1}",
                feature.Name,
                feature.Value);
        }

        public void Visit<T>(IFeature<T> feature)
        {
            Contract.Requires(feature != null);

            // can't specify constraints to narrow for enums
            var valueType = typeof(T);
            if (valueType.IsEnum)
            {
                this.builder.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}{1}",
                    feature.Name,
                    Enum.GetName(valueType, feature.Value));
            }
            else if (InspectionHelper.IsNumericType(typeof(T)))
            {
                this.builder.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}:{1}",
                    feature.Name,
                    feature.Value);
            }
            else
            {
                this.builder.AppendFormat(
                  CultureInfo.InvariantCulture,
                  " {0}{1}",
                  feature.Name,
                  feature.Value);
            }
        }

        public void Visit(INamespaceSparse namespaceSparse)
        {
            Contract.Requires(namespaceSparse != null);

            this.builder.AppendFormat(
                CultureInfo.InvariantCulture,
                " |{0}{1}",
                namespaceSparse.FeatureGroup,
                namespaceSparse.Name);

            foreach (var feature in namespaceSparse.Features)
            {
                feature.Visit();
            }
        }

        public string Visit(ILabel label, IVisitableNamespace[] namespaces)
        {
            Contract.Requires(namespaces != null);

            // see https://github.com/JohnLangford/vowpal_wabbit/wiki/Input-format 
            // prefix with label
            this.builder = new StringBuilder();
            if (label != null)
            {
                builder.Append(label.ToVowpalWabbitFormat());
            }

            foreach (var n in namespaces)
            {
                n.Visit();
            }

            return this.builder.ToString();
        }
    }
}
