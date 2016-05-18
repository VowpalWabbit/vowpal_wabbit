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
using System.Linq;
using VW.Labels;
using VW.Labels;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// Build <see cref="VowpalWabbitExample"/> from JSON following https://github.com/JohnLangford/vowpal_wabbit/wiki/JSON
    /// </summary>
    public sealed class VowpalWabbitJsonBuilder : IDisposable
    {
        /// <summary>
        /// Mapping from properties to types for labels.
        /// </summary>
        private static readonly Dictionary<string, Type> labelPropertyMapping;

        private readonly VowpalWabbit vw;
        private readonly VowpalWabbitDefaultMarshaller defaultMarshaller;
        private readonly JsonSerializer jsonSerializer;
        private readonly VowpalWabbitJsonSerializer serializer;
        private readonly VowpalWabbitJsonReferenceResolver referenceResolver;
        private readonly List<string> namespaceStrings;

        private JsonReader reader;
        private ILabel label;
        private Func<string, bool> specialPropertyAction;

        static VowpalWabbitJsonBuilder()
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

        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitJsonBuilder"/>.
        /// </summary>
        public VowpalWabbitJsonBuilder(VowpalWabbitJsonSerializer serializer, IVowpalWabbitExamplePool vwPool, VowpalWabbitDefaultMarshaller defaultMarshaller, JsonSerializer jsonSerializer)
        {
            Contract.Requires(serializer != null);
            Contract.Requires(vw != null);
            Contract.Requires(defaultMarshaller != null);
            Contract.Requires(jsonSerializer != null);

            this.namespaceStrings = new List<string>();
            this.referenceResolver = serializer.ReferenceResolver;
            this.serializer = serializer;
            this.vw = vwPool.Native;
            this.defaultMarshaller = defaultMarshaller;
            this.jsonSerializer = jsonSerializer;

            this.DefaultNamespaceContext = new VowpalWabbitMarshalContext(this.vw);
        }

        // useful for tracking down bugs
        // private string DefaultNamespaceContextStackTrace;

        /// <summary>
        /// The marshalling context for the default namespace. Can be modified until <see cref="CreateExample"/>.
        /// </summary>
        public VowpalWabbitMarshalContext DefaultNamespaceContext { get; private set; }

        /// <summary>
        /// Creates the managed example representation.
        /// </summary>
        /// <returns>Returns the managed example.</returns>
        public VowpalWabbitExample CreateExample()
        {
            try
            {
                var vwExample = this.DefaultNamespaceContext.ExampleBuilder.CreateExample();

                if (this.vw.Settings.EnableStringExampleGeneration)
                {
                    var str = this.DefaultNamespaceContext.ToString();
                    if (str.Length > 0)
                        this.namespaceStrings.Insert(0, str);

                    vwExample.VowpalWabbitString = string.Join(" ", this.namespaceStrings);
                }

                return vwExample;
            }
            finally
            {
                // useful for tracking down bugs
                // this.DefaultNamespaceContextStackTrace = "Create Example" + Environment.StackTrace;
                this.DefaultNamespaceContext.Dispose();
                this.DefaultNamespaceContext = null;
            }
        }

        /// <summary>
        /// Parses the example.
        /// </summary>
        /// <param name="reader">The example to parse.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="reader"/>.
        /// If null, <paramref name="reader"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <param name="specialPropertyAction">Action to be executed when sepcial properties are discovered.</param>
        /// <returns>The VowpalWabbit native example.</returns>
        public void Parse(JsonReader reader, ILabel label = null, Func<string, bool> specialPropertyAction = null)
        {
            // avoid parameter passing for the sake of non-reentrantness
            this.reader = reader;
            this.label = label;
            this.specialPropertyAction = specialPropertyAction;

            if (label != null)
                this.defaultMarshaller.MarshalLabel(this.DefaultNamespaceContext, label);

            // handle the case when the reader is already positioned at JsonToken.StartObject
            if (reader.TokenType == JsonToken.None && !reader.Read())
                return;

            if (reader.TokenType != JsonToken.StartObject)
                throw new VowpalWabbitJsonException(reader.Path,
                    string.Format("Expected start object. Found '{0}' and value '{1}'",
                    reader.TokenType, reader.Value));

            var ns = new Namespace(this.vw);
            var path = new List<LocalContext>
                {
                    new LocalContext
                    {
                        Namespace = ns,
                        Context = this.DefaultNamespaceContext,
                        JsonProperty = string.Empty
                    }
                };

            this.defaultMarshaller.MarshalNamespace(this.DefaultNamespaceContext, ns, () => this.ParseProperties(path));
        }

        private void ParseSpecialProperty(LocalContext context, string propertyName)
        {
            var propertyConfiguration = this.vw.Settings.PropertyConfiguration;

            // special fields
            if (propertyName == propertyConfiguration.LabelProperty)
            {
                // passed in label has precedence
                if (label == null)
                    this.ParseLabel();
                else
                    reader.Skip();
            }
            else if (propertyName == propertyConfiguration.TextProperty)
            {
                // parse text segment feature
                this.defaultMarshaller.MarshalFeatureStringSplit(
                    context.Context,
                    context.Namespace,
                    new Feature(propertyName),
                    reader.ReadAsString());
            }
            else
            {
                // forward to handler
                if (specialPropertyAction == null || !specialPropertyAction(propertyName))
                    reader.Skip(); // if not handled, skip it
            }
         }

        private void ParseLabel()
        {
            // peak the first property name
            if (!reader.Read())
                throw new VowpalWabbitJsonException(reader.Path, "Unexpected end");

            switch (reader.TokenType)
            {
                case JsonToken.StartObject:
                    {
                        // parse complex object
                        if (!reader.Read() || reader.TokenType != JsonToken.PropertyName)
                            throw new VowpalWabbitJsonException(reader.Path, "Expected at least a single property to determine the label object");

                        var propertyName = (string)reader.Value;

                        var prefixReader = new PrefixedJsonReader(this.reader,
                            Tuple.Create(JsonToken.StartObject, (object)null),
                            Tuple.Create(JsonToken.PropertyName, (object)propertyName));

                        Type labelType;
                        if (!labelPropertyMapping.TryGetValue(propertyName, out labelType))
                            throw new VowpalWabbitJsonException(reader.Path, "The first property ('" + propertyName + "') must match to a property of a VowpalWabbit label type.");

                        var label = (ILabel)jsonSerializer.Deserialize(prefixReader, labelType);

                        this.defaultMarshaller.MarshalLabel(this.DefaultNamespaceContext, label);
                    }
                    break;
                case JsonToken.Integer:
                case JsonToken.Float:
                case JsonToken.String:
                    {
                        // pass label directly to VW
                        var labelString = reader.Value.ToString();

                        this.defaultMarshaller.MarshalLabel(this.DefaultNamespaceContext, new StringLabel(labelString));
                    }
                    break;
                case JsonToken.Null:
                    // ignore
                    break;
                default:
                    throw new VowpalWabbitJsonException(reader.Path, "Expected label object");
            }
        }

        /// <summary>
        /// Expects that actual feature value.
        /// </summary>
        private void ParseFeature(List<LocalContext> path, string featureName)
        {
            switch (featureName)
            {
                case "$id":
                    {
                        if (this.referenceResolver == null)
                            return;

                        var id = (string)reader.Value;

                        if (!reader.Read() ||
                            reader.TokenType != JsonToken.PropertyName ||
                            (string)reader.Value != "$values")
                            throw new VowpalWabbitJsonException(reader.Path, "Expecting '$values' property");

                        // read $values
                        if (!reader.Read())
                            throw new VowpalWabbitJsonException(reader.Path, "Unexpected end");

                        // create re-useable marshalling call
                        var marshalAction = this.ParseFeatureReUsable();

                        // keep action for re-use
                        this.referenceResolver.AddReference(id, marshalAction);

                        // go up 2 levels to find actual namespace, the last one is actually the property we want to serialize
                        featureName = path.Last().JsonProperty;
                        var context = path[path.Count - 2];
                        marshalAction.Marshal(this.defaultMarshaller, context.Context, context.Namespace, featureName);
                    }
                    return;
                case "$ref":
                    {
                        if (this.referenceResolver == null)
                            return;

                        var id = (string)reader.Value;

                        // go up 2 levels to find actual namespace, the last one is actually the property we want to serialize
                        featureName = path.Last().JsonProperty;
                        var ns = path[path.Count - 2].Namespace;

                        this.referenceResolver.Resolve(
                            this.serializer,
                            id,
                            marshalAction =>
                            {
                                // setup fresh context
                                using (var context = new VowpalWabbitMarshalContext(this.vw, this.DefaultNamespaceContext.ExampleBuilder))
                                {
                                    this.defaultMarshaller.MarshalNamespace(
                                        context,
                                        ns,
                                        () => marshalAction.Marshal(this.defaultMarshaller, context, ns, featureName));

                                    // append default namespaces features if we found some
                                    if (this.vw.Settings.EnableStringExampleGeneration)
                                    {
                                        var str = context.ToString();
                                        if (str.Length > 0)
                                            this.namespaceStrings.Add(str);
                                    }
                                }
                            });
                    }
                    return;
            }

            var localContext = path.Last();
            this.ParseFeature(localContext.Context, localContext.Namespace, featureName);
        }

        private IVowpalWabbitMarshalAction ParseFeatureReUsable()
        {
            // make sure the returned action is independent of the current parsing context, so we can ship it
            switch (reader.TokenType)
            {
                case JsonToken.Float:
                    return VowpalWabbitMarshalActions.Create((double)reader.Value);
                case JsonToken.Integer:
                    return VowpalWabbitMarshalActions.Create((long)reader.Value);
                case JsonToken.String:
                    return VowpalWabbitMarshalActions.Create((string)reader.Value);
                case JsonToken.Boolean:
                    return VowpalWabbitMarshalActions.Create((bool)reader.Value);
                case JsonToken.Comment:
                case JsonToken.Null:
                    // probably best to ignore?
                    break;
                case JsonToken.StartArray:
                    return this.ParseFeatureArrayReUsable();
            }

            return null;
        }

        /// <summary>
        /// Expects: "1,2.2,3]" (excluding the leading [)
        /// </summary>
        private IVowpalWabbitMarshalAction ParseFeatureArrayReUsable()
        {
            var values = new float[16];
            var index = 0;
            while (reader.Read())
            {
                float val;
                switch (reader.TokenType)
                {
                    case JsonToken.Integer:
                        val = (float)(long)reader.Value;
                        break;
                    case JsonToken.Float:
                        val = (float)(double)reader.Value;
                        break;
                    case JsonToken.EndArray:
                        goto done;
                    default:
                        throw new VowpalWabbitJsonException(reader.Path, "Unxpected token " + reader.TokenType + " while deserializing dense feature array");
                }

                if (index == values.Length)
                {
                    var newValues = new float[values.Length * 2];
                    Array.Copy(values, newValues, values.Length);
                    values = newValues;
                }

                values[index++] = val;
            }
        done:

            return VowpalWabbitMarshalActions.Create(values, index);
        }

        /// <summary>
        /// Expects that actual feature value.
        /// </summary>
        private void ParseFeature(VowpalWabbitMarshalContext context, Namespace ns, string featureName)
        {
            switch (reader.TokenType)
            {
                case JsonToken.Float:
                    VowpalWabbitMarshalActions.Marshal(this.defaultMarshaller, context, ns, featureName, (double)reader.Value);
                    break;
                case JsonToken.Integer:
                    VowpalWabbitMarshalActions.Marshal(this.defaultMarshaller, context, ns, featureName, (long)reader.Value);
                    break;
                case JsonToken.String:
                    VowpalWabbitMarshalActions.Marshal(this.defaultMarshaller, context, ns, featureName, (string)reader.Value);
                    break;
                case JsonToken.Boolean:
                    VowpalWabbitMarshalActions.Marshal(this.defaultMarshaller, context, ns, featureName, (bool)reader.Value);
                    break;
                case JsonToken.Comment:
                case JsonToken.Null:
                    // probably best to ignore?
                    break;
                case JsonToken.StartArray:
                    this.ParseFeatureArray(context, ns);
                    break;
                default:
                    throw new VowpalWabbitJsonException(reader.Path, "Unexpected token " + reader.TokenType + " while deserializing primitive feature");
            }
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
                        MarshalFloatFeature(context, ns, index, (float)(long)reader.Value);
                        break;
                    case JsonToken.Float:
                        MarshalFloatFeature(context, ns, index, (float)(double)reader.Value);
                        break;
                    case JsonToken.EndArray:
                        return;
                    default:
                        throw new VowpalWabbitJsonException(reader.Path, "Unxpected token " + reader.TokenType + " while deserializing dense feature array");
                }
                index++;
            }
        }

        private static void MarshalFloatFeature(VowpalWabbitMarshalContext context, Namespace ns, ulong index, float value)
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
        /// Parses { "feature1":1, "feature2":true, .... }
        /// </summary>
        private void ParseNamespaceAndFeatures(List<LocalContext> path, string namespaceValue)
        {
            LocalContext localContext = null;

            try
            {
                var ns = new Namespace(this.vw, namespaceValue);
                localContext = new LocalContext
                {
                    Namespace = ns,
                    Context = new VowpalWabbitMarshalContext(this.vw, this.DefaultNamespaceContext.ExampleBuilder),
                    JsonProperty = namespaceValue
                };

                path.Add(localContext);

                var propertyConfiguration = this.vw.Settings.PropertyConfiguration;
                this.defaultMarshaller.MarshalNamespace(localContext.Context, ns, () => this.ParseProperties(path));

                path.RemoveAt(path.Count - 1);

                // append default namespaces features if we found some
                if (this.vw.Settings.EnableStringExampleGeneration)
                {
                    var str = localContext.Context.ToString();
                    if (str.Length > 0)
                        this.namespaceStrings.Add(str);
                }
            }
            finally
            {
                if (localContext != null && localContext.Context != null)
                    localContext.Context.Dispose();
            }
        }

        private void ParseProperties(List<LocalContext> path)
        {
            var propertyConfiguration = this.vw.Settings.PropertyConfiguration;

            while (reader.Read())
            {
                switch (reader.TokenType)
                {
                    case JsonToken.PropertyName:
                        var propertyName = (string)reader.Value;
                        if (propertyName.StartsWith(propertyConfiguration.FeatureIgnorePrefix, StringComparison.Ordinal) ||
                            propertyConfiguration.IsSpecialProperty(propertyName))
                        {
                            this.ParseSpecialProperty(path.Last(), propertyName);
                            continue;
                        }

                        if (!reader.Read())
                            throw new VowpalWabbitJsonException(reader.Path, "Unexpected end while parsing namespace");

                        // TODO: this.Context might have to be a stack...
                        if (reader.TokenType == JsonToken.StartObject)
                            this.ParseNamespaceAndFeatures(path, propertyName);
                        else
                            this.ParseFeature(path, propertyName);
                        break;
                    case JsonToken.EndObject:
                        return;
                }
            }
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.DefaultNamespaceContext != null)
                {
                    // useful for tracking down bugs
                    // this.DefaultNamespaceContextStackTrace = "Dispose" + Environment.StackTrace;
                    this.DefaultNamespaceContext.Dispose();
                    this.DefaultNamespaceContext = null;
                }
            }
        }

        private sealed class LocalContext
        {
            internal VowpalWabbitMarshalContext Context { get; set; }

            internal Namespace Namespace { get; set; }

            internal string JsonProperty { get; set; }
        }
    }
}
