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
using System.Text.RegularExpressions;
using VW.Labels;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// The default marshaller for most types supported by VW.
    /// </summary>
    public sealed partial class VowpalWabbitDefaultMarshaller
    {
        /// <summary>
        /// Singleton default marshaller as it is stateless.
        /// </summary>
        public static readonly VowpalWabbitDefaultMarshaller Instance = new VowpalWabbitDefaultMarshaller();

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

            context.AppendStringExample(feature.Dictify, " {0}", feature.Name);
        }

        /// <summary>
        /// Marshals an enum value into native VW.
        /// </summary>
        /// <typeparam name="T">The enum type.</typeparam>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        /// <example>Gender = Male yields "GenderMale" in VW native string format.</example>
        public void MarshalEnumFeature<T>(VowpalWabbitMarshalContext context, Namespace ns, EnumerizedFeature<T> feature, T value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            context.NamespaceBuilder.AddFeature(feature.FeatureHash(value), 1f);

            context.AppendStringExample(feature.Dictify, " {0}{1}", feature.Name, value);
        }

        /// <summary>
        /// Marshals any type into native VW, by constructing a 1-hot encoding using <see cref="object.ToString()"/>.
        /// </summary>
        /// <typeparam name="T">The type to be enumerized.</typeparam>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        /// <example><typeparamref name="T"/> is <see cref="System.Int32"/>, actual value '25' and <see cref="Feature.Name"/> is 'Age'.
        /// The result is equivalent to 'Age25'.
        /// </example>
        public void MarshalEnumerizeFeature<T>(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, T value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            var stringValue = feature.Name + value.ToString();
            context.NamespaceBuilder.AddFeature(context.VW.HashFeature(stringValue, ns.NamespaceHash), 1f);

            context.AppendStringExample(feature.Dictify, " {0}", stringValue);
        }

        private static Regex escapeCharacters = new Regex("[ \t|:]", RegexOptions.Compiled);

        /// <summary>
        /// Marshals the supplied string into VW native space. Spaces are escaped using '_'.
        /// Only <paramref name="value"/> is serialized, <paramref name="feature"/> Name is ignored.
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        /// <example><paramref name="value"/> is "New York". Result is "New_York".</example>
        public void MarshalFeatureStringEscape(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, string value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (string.IsNullOrWhiteSpace(value))
                return;

            // safe escape spaces
            value = escapeCharacters.Replace(value, "_");

            var featureHash = context.VW.HashFeature(value, ns.NamespaceHash);
            context.NamespaceBuilder.AddFeature(featureHash, 1f);

            context.AppendStringExample(feature.Dictify, " {0}", value);
        }

        /// <summary>
        /// Marshals the supplied string into VW native space. Spaces are escaped using '_'. Includes the <see cref="Feature.Name"/> in the 1-hot encoded feature.
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        /// <example><paramref name="value"/> is "New York". <paramref name="feature"/> Name is "Location". Result is "LocationNew_York".</example>
        public void MarshalFeatureStringEscapeAndIncludeName(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, string value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (string.IsNullOrWhiteSpace(value))
                return;

            // safe escape spaces
            value = feature.Name + escapeCharacters.Replace(value, "_");

            var featureHash = context.VW.HashFeature(value, ns.NamespaceHash);
            context.NamespaceBuilder.AddFeature(featureHash, 1f);

            context.AppendStringExample(feature.Dictify, " {0}", value);
        }

        /// <summary>
        /// Marshals the supplied string into VW native space, by splitting the word by white space.
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        /// <example><paramref name="value"/> is "New York". Result is "New York", corresponding to 2 featuers in VW native space.</example>
        public void MarshalFeatureStringSplit(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, string value)
        {
            if (string.IsNullOrWhiteSpace(value))
                return;

            var words = value.Split((char[])null, StringSplitOptions.RemoveEmptyEntries);
            foreach (var s in words)
            {
                var featureHash = context.VW.HashFeature(escapeCharacters.Replace(s, "_"), ns.NamespaceHash);
                context.NamespaceBuilder.AddFeature(featureHash, 1f);
            }

            if (context.StringExample == null)
            {
                return;
            }

            foreach (var s in words)
            {
                context.AppendStringExample(feature.Dictify, " {0}", escapeCharacters.Replace(s, "_"));
            }
        }

        /// <summary>
        /// Transfers feature data to native space.
        /// </summary>
        /// <typeparam name="TKey"></typeparam>
        /// <typeparam name="TValue"></typeparam>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
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
                        Convert.ToSingle(kvp.Value, CultureInfo.InvariantCulture));
            }

            if (context.StringExample == null)
            {
                return;
            }

            foreach (var kvp in value)
            {
                context.AppendStringExample(
                    feature.Dictify,
                    " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                    Convert.ToString(kvp.Key),
                    Convert.ToSingle(kvp.Value, CultureInfo.InvariantCulture));
            }
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
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
                    Convert.ToSingle(item.Value, CultureInfo.InvariantCulture));
            }

            if (context.StringExample == null)
            {
                return;
            }

            foreach (DictionaryEntry item in value)
            {
                context.AppendStringExample(
                    feature.Dictify,
                    " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                    Convert.ToString(item.Key),
                    Convert.ToSingle(item.Value, CultureInfo.InvariantCulture));
            }
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IEnumerable<string> value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (value == null)
                return;

            foreach (var item in value)
                context.NamespaceBuilder.AddFeature(context.VW.HashFeature(item.Replace(' ', '_'), ns.NamespaceHash), 1f);

            if (context.StringExample == null)
                return;

            foreach (var item in value)
                context.AppendStringExample(feature.Dictify, " {0}", item);
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="feature">The feature description.</param>
        /// <param name="value">The actual feature value.</param>
        public void MarshalFeature(VowpalWabbitMarshalContext context, Namespace ns, Feature feature, IVowpalWabbitSerializable value)
        {
            Contract.Requires(context != null);
            Contract.Requires(ns != null);
            Contract.Requires(feature != null);

            if (value == null)
                return;

            value.Marshal(context, ns, feature);
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="ns">The namespace description.</param>
        /// <param name="featureVisits"></param>
        public int MarshalNamespace(VowpalWabbitMarshalContext context, Namespace ns, Action featureVisits)
        {
            try
            {
                // the namespace is only added on dispose, to be able to check if at least a single feature has been added
                context.NamespaceBuilder = context.ExampleBuilder.AddNamespace(ns.FeatureGroup);

                var position = 0;
                var stringExample = context.StringExample;
                if (context.StringExample != null)
                    position = stringExample.Append(ns.NamespaceString).Length;

                featureVisits();

                if (context.StringExample != null)
                {
                    if (position == stringExample.Length)
                        // no features added, remove namespace
                        stringExample.Length = position - ns.NamespaceString.Length;
                }

                return (int)context.NamespaceBuilder.FeatureCount;
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

        /// <summary>
        ///
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="label"></param>
        public void MarshalLabel(VowpalWabbitMarshalContext context, ILabel label)
        {
            if (label == null)
                return;

            context.ExampleBuilder.ApplyLabel(label);

            // prefix with label
            if (context.StringExample != null)
                context.StringLabel = label.ToString();
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="context">The marshalling context.</param>
        /// <param name="label"></param>
        public void MarshalLabel(VowpalWabbitMarshalContext context, string label)
        {
            if (label == null)
                return;

            context.ExampleBuilder.ApplyLabel(new StringLabel(label));

            context.StringLabel = label;
        }
    }
}
