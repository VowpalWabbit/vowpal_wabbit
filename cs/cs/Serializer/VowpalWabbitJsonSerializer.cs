// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJsonSerializer.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Globalization;
using System.IO;
using System.Linq;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// A deserializer from JSON to Vowpal Wabbit native examples.
    /// </summary>
    public sealed class VowpalWabbitJsonSerializer
    {
        /// <summary>
        /// Mapping from properties to types for labels.
        /// </summary>
        private static readonly Dictionary<string, Type> labelPropertyMapping;

        static VowpalWabbitJsonSerializer()
        {
            // find mapping from property names to types
            var q = from t in new[] { typeof(SimpleLabel), typeof(ContextualBanditLabel) }
                    from p in t.GetProperties()
                    let jsonProperty = (JsonPropertyAttribute)p.GetCustomAttributes(typeof(JsonPropertyAttribute), true).FirstOrDefault()
                    where jsonProperty != null
                    select new
                    {
                        Type = t,
                        JsonProperty = jsonProperty,
                        Property = p
                    };

            labelPropertyMapping = q.ToDictionary(e => e.JsonProperty.PropertyName ?? e.Property.Name, e => e.Type);
        }
        private readonly VowpalWabbit vw;
        private readonly VowpalWabbitDefaultMarshaller defaultMarshaller;
        private readonly JsonSerializer jsonSerializer;
        private JsonReader reader;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJson"/> class.
        /// </summary>
        /// <param name="vw">The VW native instance.</param>
        public VowpalWabbitJsonSerializer(VowpalWabbit vw)
        {
            Contract.Requires(vw != null);

            this.vw = vw;
            this.defaultMarshaller = new VowpalWabbitDefaultMarshaller();
            this.jsonSerializer = new JsonSerializer();
        }

        /// <summary>
        /// Parses the example.
        /// </summary>
        /// <param name="json">The example to parse.</param>
        /// <returns>The VowpalWabbit native example.</returns>
        public VowpalWabbitExample Parse(string json)
        {
            using (var textReader = new JsonTextReader(new StringReader(json)))
            {
                return this.Parse(textReader);
            }
        }

        private void SkipAuxiliary()
        {
            var nestedCount = 0;
            while (reader.Read())
            {
                switch (reader.TokenType)
                {
                    case JsonToken.StartArray:
                    case JsonToken.StartConstructor:
                    case JsonToken.StartObject:
                        nestedCount++;
                        break;
                    case JsonToken.EndArray:
                    case JsonToken.EndConstructor:
                    case JsonToken.EndObject:
                        nestedCount--;
                        break;
                    default:
                        break;
                }
            }
        }

        /// <summary>
        /// Parses the example.
        /// </summary>
        /// <param name="reader">The example to parse.</param>
        /// <returns>The VowpalWabbit native example.</returns>
        public VowpalWabbitExample Parse(JsonReader reader)
        {
            // avoid parameter passing for the sake of non-reentrantness
            this.reader = reader;
            using (VowpalWabbitMarshalContext context = new VowpalWabbitMarshalContext(this.vw))
            using (VowpalWabbitMarshalContext defaultNamespaceContext = new VowpalWabbitMarshalContext(this.vw, context.ExampleBuilder))
            {
                if (!reader.Read())
                    throw new ArgumentException("json");

                if (reader.TokenType != JsonToken.StartObject)
                    throw new ArgumentException("json");

                Namespace defaultNamespace = new Namespace(this.vw);
                using (defaultNamespaceContext.NamespaceBuilder = defaultNamespaceContext.ExampleBuilder.AddNamespace(' '))
                {
                    while (reader.Read())
                    {
                        switch (reader.TokenType)
                        {
                            case JsonToken.PropertyName:
                                var propertyName = (string)reader.Value;
                                if (propertyName.StartsWith("_"))
                                {
                                    // special fields
                                    switch (propertyName)
                                    {
                                        case "_label":
                                            this.ParseLabel(context);
                                            break;
                                        //case "_shared":
                                        //    break;
                                        //case "_actions":
                                        //    break;
                                        //case "action":
                                        //    break;
                                        default:
                                            SkipAuxiliary();
                                            break;
                                    }
                                }
                                else
                                {
                                    if (!reader.Read())
                                        throw new ArgumentException("...");

                                    if (reader.TokenType == JsonToken.StartObject)
                                        this.ParseNamespaceAndFeatures(context, propertyName);
                                    else
                                        this.ParseFeature(defaultNamespaceContext, defaultNamespace, propertyName);

                                }
                                break;
                        }
                    }

                    // append default namespaces features if we found some
                    if (defaultNamespaceContext.StringExample != null && defaultNamespaceContext.StringExample.Length > 0)
                    {
                        context.StringExample.AppendFormat(CultureInfo.InvariantCulture,
                            "| {0}", defaultNamespaceContext.StringExample);
                    }
                }

                var vwExample = context.ExampleBuilder.CreateExample();

                if (this.vw.Settings.EnableStringExampleGeneration)
                    vwExample.VowpalWabbitString = context.StringExample.ToString();

                return vwExample;
            }
        }

        private void ParseLabel(VowpalWabbitMarshalContext context)
        {
            // peak the first property name
            if (!reader.Read())
                throw new ArgumentException("...");

            if (reader.TokenType != JsonToken.StartObject)
                throw new ArgumentException("...");

            if (!reader.Read())
                throw new ArgumentException("...");

            if (reader.TokenType != JsonToken.PropertyName)
                throw new ArgumentException("...");

            var propertyName = (string)reader.Value;

            var prefixReader = new PrefixedJsonReader(this.reader,
                Tuple.Create(JsonToken.StartObject, (object)null),
                Tuple.Create(JsonToken.PropertyName, (object)propertyName));

            Type labelType;
            if (!labelPropertyMapping.TryGetValue(propertyName, out labelType))
                throw new ArgumentException("...");

            var label = (ILabel)jsonSerializer.Deserialize(prefixReader, labelType);

            this.defaultMarshaller.MarshalLabel(context, label);
        }

        /// <summary>
        /// Expects: "1,2.2,3]" (excluding the leading [)
        /// </summary>
        private void ParseFeatureArray(VowpalWabbitMarshalContext context, Namespace ns)
        {
            ulong index = 0;

            while (reader.Read())
            {
                switch (reader.TokenType)
                {
                    case JsonToken.Integer:
                        this.MarshalFloatFeature(context, ns, index, (float)(long)reader.Value);
                        break;
                    case JsonToken.Float:
                        this.MarshalFloatFeature(context, ns, index, (float)(double)reader.Value);
                        break;
                    case JsonToken.EndArray:
                        return;
                    default:
                        throw new ArgumentException("...");
                }
                index++;
            }
        }

        private void MarshalFloatFeature(VowpalWabbitMarshalContext context, Namespace ns, ulong index, float value)
        {
            context.NamespaceBuilder.AddFeature(ns.NamespaceHash + index, value);

            if (context.StringExample != null)
            {
                context.AppendStringExample(
                    false,
                    " {0}:" + (context.VW.Settings.EnableStringFloatCompact ? "{1}" : "{1:E20}"),
                    index,
                    value);
            }
        }

        /// <summary>
        /// Expects that actual feature value.
        /// </summary>
        private void ParseFeature(VowpalWabbitMarshalContext context, Namespace ns, string featureName)
        {
            switch (reader.TokenType)
            {
                case JsonToken.Float:
                    {
                        var feature = new PreHashedFeature(this.vw, ns, featureName);
                        this.defaultMarshaller.MarshalFeature(context, ns, feature, (double)reader.Value);
                    }
                    break;
                case JsonToken.Integer:
                    {
                        var feature = new PreHashedFeature(this.vw, ns, featureName);
                        this.defaultMarshaller.MarshalFeature(context, ns, feature, (long)reader.Value);
                    }
                    break;
                case JsonToken.String:
                    {
                        var feature = new Feature(featureName);
                        this.defaultMarshaller.MarshalFeature(context, ns, feature, (string)reader.Value);
                    }
                    break;
                case JsonToken.Boolean:
                    {
                        var feature = new PreHashedFeature(this.vw, ns, featureName);
                        this.defaultMarshaller.MarshalFeature(context, ns, feature, (bool)reader.Value);
                    }
                    break;
                case JsonToken.Comment:
                case JsonToken.Null:
                    // probably best to ignore?
                    break;
                case JsonToken.StartArray:
                    this.ParseFeatureArray(context, ns);
                    break;
                default:
                    throw new ArgumentException("...");
            }

        }

        /// <summary>
        /// Parses { "feature1":1, "feature2":true, .... }
        /// </summary>
        private void ParseNamespaceAndFeatures(VowpalWabbitMarshalContext context, string namespaceValue)
        {
            var ns = new Namespace(this.vw, namespaceValue);
            this.defaultMarshaller.MarshalNamespace(context, ns, () =>
            {
                while (reader.Read())
                {
                    switch (reader.TokenType)
                    {
                        case JsonToken.PropertyName:
                            var featureName = (string)reader.Value;

                            if (!reader.Read())
                                throw new ArgumentException("...");

                            this.ParseFeature(context, ns, featureName);
                            break;
                        case JsonToken.EndObject:
                            return;
                    }
                }
            });
        }
    }
}
