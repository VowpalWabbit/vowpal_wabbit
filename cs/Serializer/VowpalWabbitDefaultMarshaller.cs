// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitDefaultMarshaller.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    public partial class VowpalWabbitDefaultMarshaller
    {
        private readonly bool disableStringExampleGeneration;

        public VowpalWabbitDefaultMarshaller()
        {
        }

        public VowpalWabbitDefaultMarshaller(bool disableStringExampleGeneration)
        {
            this.disableStringExampleGeneration = disableStringExampleGeneration;
        }

        public void MarshalFeature<T>(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, T value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            // TODO: handle enum
            var featureString = feature.Name + Convert.ToString(value);
            var featureHash = context.VW.HashFeature(featureString, ns.NamespaceHash);

            context.NamespaceBuilder.AddFeature(featureHash, 1f);

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            context.StringExample.AppendFormat(
                CultureInfo.InvariantCulture,
                " {0}",
                featureString);
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void MarshalFeature<TKey, TValue>(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IEnumerable<KeyValuePair<TKey, TValue>> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (value == null)
            {
                return;
            }

            foreach (var kvp in value)
            {
                context.NamespaceBuilder.AddFeature(
                        context.VW.HashFeature(Convert.ToString(kvp.Key), ns.NamespaceHash),
                        (float)Convert.ToDouble(kvp.Value));
            }

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            foreach (var kvp in value)
            {
                // TODO: not sure if negative numbers will work
                context.StringExample.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}:{1}",
                    Convert.ToString(kvp.Key),
                    (float)Convert.ToDouble(kvp.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IDictionary value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (value == null)
            {
                return;
            }

            foreach (DictionaryEntry item in value)
            {
                context.NamespaceBuilder.AddFeature(
                    context.VW.HashFeature(Convert.ToString(item.Key), ns.NamespaceHash),
                    (float)Convert.ToDouble(item.Value));
            }

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            foreach (DictionaryEntry item in value)
            {
                context.StringExample.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}:{1}",
                    Convert.ToString(item.Key),
                    (float)Convert.ToDouble(item.Value));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <param name="feature">The feature.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IEnumerable<string> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (value == null)
            {
                return;
            }

            foreach (var item in value)
            {
                context.NamespaceBuilder.AddFeature(context.VW.HashFeature(item, ns.NamespaceHash), 1f);
            }

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            foreach (var item in value)
            {
                context.StringExample.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}",
                    item);
            }
        }

        public void MarshalNamespace(VowpalWabbitMarshalContext context, Namespace ns, Action featureVisits)
        {
            try
            {
                // the namespace is only added on dispose, to be able to check if at least a single feature has been added
                context.NamespaceBuilder = context.ExampleBuilder.AddNamespace(ns.FeatureGroup);

                var position = 0;
                var stringExample = context.StringExample;
                if (!this.disableStringExampleGeneration)
                {
                    position = stringExample.Append(ns.NamespaceString).Length;
                }

                featureVisits();

                if (!this.disableStringExampleGeneration)
                {
                    if (position == stringExample.Length)
                    {
                        // no features added, remove namespace
                        stringExample.Length = position - ns.NamespaceString.Length;
                    }
                }
            }
            finally
            {
                if (context.NamespaceBuilder != null)
                {
                    context.NamespaceBuilder.Dispose();
                    context.NamespaceBuilder = null;
                }
            }
        }

        public void MarshalLabel(VowpalWabbitMarshalContext context, ILabel label)
        {
            if (label == null)
            {
                return;
            }

            context.ExampleBuilder.ParseLabel(label.ToVowpalWabbitFormat());

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            // prefix with label
            context.StringExample.Append(label.ToVowpalWabbitFormat());
        }
    }
}
