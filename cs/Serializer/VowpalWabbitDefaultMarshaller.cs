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
    /// <summary>
    /// The default marshaller for most types supported by VW.
    /// </summary>
    public partial class VowpalWabbitDefaultMarshaller
    {
        private readonly bool disableStringExampleGeneration;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitDefaultMarshaller"/> class.
        /// </summary>
        /// <param name="disableStringExampleGeneration">True if no vw native strings should be generated, false otherwise.</param>
        public VowpalWabbitDefaultMarshaller(bool disableStringExampleGeneration)
        {
            this.disableStringExampleGeneration = disableStringExampleGeneration;
        }

        /// <summary>
        /// Marshals the given value <paramref name="value"/> into native VW by
        ///
        /// <list type="number">
        /// <item><description>string concatenating the feature name and the value.</description></item>
        /// <item><description>Hashing of the resulting string.</description></item>
        /// </list>
        ///
        /// e.g. feature name = "Age", value = 25 yields "Age25:1" in VW native string format.
        /// </summary>
        /// <typeparam name="T">The value type.</typeparam>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        /// <remarks>This is a fallback method if no other types match.</remarks>
        public void MarshalFeature<T>(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, T value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

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
        /// Marshals a boolean value into native VW.
        /// 
        /// e.g. loggedIn = true yields "loggedIn" in VW native string format.
        /// e.g. loggedIn = false yields an empty string.
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, PreHashedFeature feature, bool value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (!value)
            {
                return;
            }

            context.NamespaceBuilder.AddFeature(feature.FeatureHash, 1f);

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            context.StringExample.AppendFormat(
                CultureInfo.InvariantCulture,
                " {0}",
                feature.Name);
        }

        /// <summary>
        /// Marshals an enum value into native VW.
        /// 
        /// e.g. Gender = Male yields "GenderMale" in VW native string format.
        /// </summary>
        /// <typeparam name="T">The enum type.</typeparam>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        public void MarshalEnumFeature<T>(VowpalWabbitMarshalContext context, Namespace ns, EnumerizedFeature<T> feature, T value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            context.NamespaceBuilder.AddFeature(feature.FeatureHash(value), 1f);

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            context.StringExample.AppendFormat(
                CultureInfo.InvariantCulture,
                " {0}{1}",
                feature.Name,
                value);
        }

        public void MarshalEnumerizeFeature<T>(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, T value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            var stringValue = feature.Name + value.ToString();
            context.NamespaceBuilder.AddFeature(context.VW.HashFeature(stringValue, ns.NamespaceHash), 1f);

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            context.StringExample.Append(' ').Append(stringValue);
        }

        public void MarshalFeatureStringEscape(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, string value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (string.IsNullOrWhiteSpace(value))
                return;

            // safe escape spaces
            value = value.Replace(' ', '_');

            var featureHash = context.VW.HashFeature(value, ns.NamespaceHash);
            context.NamespaceBuilder.AddFeature(featureHash, 1f);

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            context.StringExample.Append(' ').Append(value);
        }

        public void MarshalFeatureStringSplit(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, string value)
        {
            if (string.IsNullOrWhiteSpace(value))
                return;

            var words = value.Split((char[])null, StringSplitOptions.RemoveEmptyEntries);
            foreach (var s in words)
            {
                var featureHash = context.VW.HashFeature(s, ns.NamespaceHash);
                context.NamespaceBuilder.AddFeature(featureHash, 1f);
            }

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            foreach (var s in words)
            {
                context.StringExample.Append(' ').Append(s);
            }
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
                        (float)Convert.ToDouble(kvp.Value, CultureInfo.InvariantCulture));
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
                    " {0}:{1:E20}",
                    Convert.ToString(kvp.Key),
                    (float)Convert.ToDouble(kvp.Value, CultureInfo.InvariantCulture));
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
                    (float)Convert.ToDouble(item.Value, CultureInfo.InvariantCulture));
            }

            if (this.disableStringExampleGeneration)
            {
                return;
            }

            foreach (DictionaryEntry item in value)
            {
                context.StringExample.AppendFormat(
                    CultureInfo.InvariantCulture,
                    " {0}:{1:E20}",
                    Convert.ToString(item.Key),
                    (float)Convert.ToDouble(item.Value, CultureInfo.InvariantCulture));
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
                context.NamespaceBuilder.AddFeature(context.VW.HashFeature(item.Replace(' ', '_'), ns.NamespaceHash), 1f);
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
