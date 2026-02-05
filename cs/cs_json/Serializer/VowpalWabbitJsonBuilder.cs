// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJsonSerializer.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
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

        // required for reference resolution
        private readonly VowpalWabbitJsonSerializer serializer;
        private readonly VowpalWabbitJsonReferenceResolver referenceResolver;
        private readonly List<string> namespaceStrings;

        private JsonReader reader;

        private bool foundMulti;
        private JObject labelObject;

        private ILabel label;
        private int featureCount;

        private VowpalWabbitJsonParseState extensionState;
        private List<VowpalWabbitJsonExtension> extensions;

        static VowpalWabbitJsonBuilder()
        {
            // find mapping from property names to types
            // Note: ContinuousActionLabel is handled separately via _label_ca property detection
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

            labelPropertyMapping = q.ToDictionary(
                e => (e.JsonProperty.PropertyName ?? e.Property.Name).ToLowerInvariant(),
                e => e.Type);
        }

        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitJsonBuilder"/>.
        /// </summary>
        public VowpalWabbitJsonBuilder(IVowpalWabbitExamplePool vwPool, VowpalWabbitDefaultMarshaller defaultMarshaller, JsonSerializer jsonSerializer, int multiIndex = -1)
            : this(null, vwPool, defaultMarshaller, jsonSerializer, multiIndex)
        {
        }

        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitJsonBuilder"/>.
        /// </summary>
        public VowpalWabbitJsonBuilder(VowpalWabbitJsonSerializer serializer, IVowpalWabbitExamplePool vwPool, VowpalWabbitDefaultMarshaller defaultMarshaller, JsonSerializer jsonSerializer, int multiIndex = -1)
        {
            Debug.Assert(vwPool != null);
            Debug.Assert(defaultMarshaller != null);
            Debug.Assert(jsonSerializer != null);

            this.extensionState = new VowpalWabbitJsonParseState
            {
                JsonBuilder = this,
                VW = vwPool.Native,
                MultiIndex = multiIndex
            };

            this.namespaceStrings = new List<string>();
            this.foundMulti = false;
            if (serializer != null)
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
        /// The index the label was assigned to for multi line examples.
        /// </summary>
        public int LabelIndex { get; private set; }

        /// <summary>
        /// The label that was deserialized.
        /// </summary>
        public ILabel Label { get; private set; }

        /// <summary>
        /// Creates the managed example representation.
        /// </summary>
        /// <returns>Returns the managed example.</returns>
        public VowpalWabbitExample CreateExample()
        {
            try
            {
                if (this.featureCount == 0)
                {
                    return null;
                }

                var vwExample = this.DefaultNamespaceContext.ExampleBuilder.CreateExample();

                if (this.vw.Settings.EnableStringExampleGeneration)
                {
                    var str = this.DefaultNamespaceContext.ToString();
                    if (str.Length > 0)
                        this.namespaceStrings.Insert(0, str);

                    vwExample.VowpalWabbitString = string.Join(" ", this.namespaceStrings);
                }

                Debug.Assert(vwExample != null, "vwExample is null");
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

        // re-entering from extension
        internal void Parse(List<VowpalWabbitJsonParseContext> path, VowpalWabbitMarshalContext namespaceContext, Namespace ns)
        {
            this.featureCount = this.defaultMarshaller.MarshalNamespace(namespaceContext, ns, () => this.ParseProperties(path)) + this.featureCount;
        }

        /// <summary>
        /// Parse VW JSON
        /// </summary>
        public void Parse(JsonReader reader, VowpalWabbitMarshalContext context, Namespace ns, List<VowpalWabbitJsonExtension> extensions = null)
        {
            this.namespaceStrings.Clear();
            this.reader = reader;
            this.extensions = extensions;

            // handle the case when the reader is already positioned at JsonToken.StartObject
            if (reader.TokenType == JsonToken.None && !reader.Read())
                return;

            // don't barf on null values.
            if (reader.TokenType == JsonToken.Null)
                return;

            if (reader.TokenType != JsonToken.StartObject)
                throw new VowpalWabbitJsonException(this.reader,
                    $"Expected start object. Found '{reader.TokenType}' and value '{reader.Value}' for namespace {ns.Name}");

            // re-direct default namespace to the one passed
            var saveDefaultNamespaceContext = this.DefaultNamespaceContext;
            try
            {
                using (this.DefaultNamespaceContext = new VowpalWabbitMarshalContext(this.vw, context.ExampleBuilder))
                {
                    VowpalWabbitJsonParseContext localContext = null;
                    try
                    {
                        // setup current namespace
                        localContext = new VowpalWabbitJsonParseContext
                        {
                            Namespace = ns,
                            Context = new VowpalWabbitMarshalContext(this.vw, context.ExampleBuilder),
                            JsonProperty = ns.Name
                        };
                        {
                            this.defaultMarshaller.MarshalNamespace(
                                localContext.Context,
                                ns,
                                () => this.ParseProperties(new List<VowpalWabbitJsonParseContext> { localContext }));

                            // append string features if we found some
                            if (this.vw.Settings.EnableStringExampleGeneration)
                            {
                                context.StringExample
                                    .Append(localContext.Context.StringExample)
                                    .Append(string.Join(" ", this.namespaceStrings));
                            }
                        }
                    }
                    finally
                    {
                        if (localContext != null && localContext.Context != null)
                        {
                            localContext.Context.Dispose();
                            localContext.Context = null;
                        }
                    }
                }
            }
            finally
            {
                this.DefaultNamespaceContext = saveDefaultNamespaceContext;
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
        /// <param name="extensions">Action to be executed when special properties are discovered.</param>
        /// <returns>The VowpalWabbit native example.</returns>
        public void Parse(JsonReader reader, ILabel label = null, List<VowpalWabbitJsonExtension> extensions = null)
        {
            this.featureCount = 0;
            this.labelObject = null;
            this.foundMulti = false;

            // avoid parameter passing for the sake of non-reentrantness
            this.reader = reader;
            this.label = label;
            this.extensions = extensions;

            if (label != null)
                this.defaultMarshaller.MarshalLabel(this.DefaultNamespaceContext, label);

            // handle the case when the reader is already positioned at JsonToken.StartObject
            if (reader.TokenType == JsonToken.None && !reader.Read())
                return;

            if (reader.TokenType != JsonToken.StartObject)
                throw new VowpalWabbitJsonException(this.reader,
                    string.Format("Expected start object. Found '{0}' and value '{1}'",
                    reader.TokenType, reader.Value));

            var ns = new Namespace(this.vw);
            var path = new List<VowpalWabbitJsonParseContext>
                {
                    new VowpalWabbitJsonParseContext
                    {
                        Namespace = ns,
                        Context = this.DefaultNamespaceContext,
                        JsonProperty = string.Empty
                    }
                };

            this.extensionState.Reader = reader;
            this.extensionState.Path = path;

            // TODO: duplicate namespace recursion to enable async
            // featureCount might be modified inside ParseProperties...
            this.featureCount = this.defaultMarshaller.MarshalNamespace(this.DefaultNamespaceContext, ns, () => this.ParseProperties(path)) + this.featureCount;

            if (this.labelObject != null)
            {
                var propertyName = ((JProperty)this.labelObject.First).Name;
                Type labelType;
                if (!labelPropertyMapping.TryGetValue(propertyName.ToLowerInvariant(), out labelType))
                    throw new VowpalWabbitJsonException(this.reader, "The first property ('" + propertyName + "') must match to a property of a VowpalWabbit label type.");

                var labelObj = (ILabel)this.labelObject.ToObject(labelType);

                if (this.foundMulti)
                    this.Label = labelObj;
                else
                    this.defaultMarshaller.MarshalLabel(this.DefaultNamespaceContext, labelObj);
            }
        }

        private void ParseSpecialProperty(VowpalWabbitJsonParseContext context, string propertyName)
        {
            var propertyConfiguration = this.vw.Settings.PropertyConfiguration;

            // special fields
            if (propertyName.Equals(propertyConfiguration.LabelProperty, StringComparison.OrdinalIgnoreCase))
            {
                // passed in label has precedence
                if (label == null)
                    this.ParseLabel();
                else
                    reader.Skip();
            }
            else if (propertyName.Equals(propertyConfiguration.TextProperty, StringComparison.OrdinalIgnoreCase))
            {
                // parse text segment feature
                this.defaultMarshaller.MarshalFeatureStringSplit(
                    context.Context,
                    context.Namespace,
                    new Feature(propertyName),
                    reader.ReadAsString());
            }
            else if (propertyName.Equals(propertyConfiguration.LabelIndexProperty, StringComparison.OrdinalIgnoreCase))
            {
                if (!this.reader.Read())
                    throw new VowpalWabbitJsonException(this.reader, "Unexpected end");

                // skip
                if (this.reader.TokenType == JsonToken.Null)
                    return;

                this.LabelIndex = (int)(long)this.reader.Value;
            }
            else if (propertyName.Equals("_label_ca", StringComparison.OrdinalIgnoreCase))
            {
                // Handle continuous action (CATS) labels: {"_label_ca": {"action": ..., "cost": ..., "pdf_value": ...}}
                if (label != null)
                {
                    reader.Skip();
                    return;
                }

                if (!this.reader.Read())
                    throw new VowpalWabbitJsonException(this.reader, "Unexpected end");

                if (this.reader.TokenType == JsonToken.Null)
                    return;

                if (this.reader.TokenType != JsonToken.StartObject)
                    throw new VowpalWabbitJsonException(this.reader, "_label_ca must be an object");

                var caLabel = (ContinuousActionLabel)jsonSerializer.Deserialize(this.reader, typeof(ContinuousActionLabel));
                if (this.foundMulti)
                    this.Label = caLabel;
                else
                    this.defaultMarshaller.MarshalLabel(this.DefaultNamespaceContext, caLabel);
            }
            else if (propertyName.StartsWith(propertyConfiguration.LabelPropertyPrefix, StringComparison.OrdinalIgnoreCase))
            {
                if (!this.reader.Read())
                    throw new VowpalWabbitJsonException(this.reader, "Unexpected end");

                // skip
                if (this.reader.TokenType == JsonToken.Null)
                    return;

                if (this.labelObject == null)
                    this.labelObject = new JObject();

                var targetPropertyName = propertyName.Substring(propertyConfiguration.LabelPropertyPrefix.Length);
                this.labelObject.Add(targetPropertyName, new JValue(this.reader.Value));
            }
            else
            {
                if (propertyName.Equals(propertyConfiguration.MultiProperty, StringComparison.Ordinal))
                    this.foundMulti = true;

                // forward to handler
                if (this.extensions != null)
                    foreach (var extension in this.extensions)
                        if (extension(this.extensionState, propertyName))
                            return;

                // if not handled, skip it
                reader.Skip();
            }
        }

        private void ParseLabel()
        {
            // peak the first property name
            if (!this.reader.Read())
                throw new VowpalWabbitJsonException(this.reader, "Unexpected end");

            switch (reader.TokenType)
            {
                case JsonToken.StartObject:
                    {
                        // parse complex object
                        if (!reader.Read() || reader.TokenType != JsonToken.PropertyName)
                            throw new VowpalWabbitJsonException(this.reader, "Expected at least a single property to determine the label object");

                        var propertyName = (string)reader.Value;

                        var prefixReader = new PrefixedJsonReader(this.reader,
                            Tuple.Create(JsonToken.StartObject, (object)null),
                            Tuple.Create(JsonToken.PropertyName, (object)propertyName));

                        Type labelType;
                        if (!labelPropertyMapping.TryGetValue(propertyName.ToLowerInvariant(), out labelType))
                            throw new VowpalWabbitJsonException(this.reader, "The first property ('" + propertyName + "') must match to a property of a VowpalWabbit label type.");

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
                    throw new VowpalWabbitJsonException(this.reader, "Expected label object");
            }
        }

        /// <summary>
        /// Expects that actual feature value.
        /// </summary>
        private void ParseFeature(List<VowpalWabbitJsonParseContext> path, string featureName)
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
                            throw new VowpalWabbitJsonException(this.reader, "Expecting '$values' property");

                        // read $values
                        if (!reader.Read())
                            throw new VowpalWabbitJsonException(this.reader, "Unexpected end");

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
                        if (this.referenceResolver == null || this.serializer == null)
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
                                    this.featureCount += this.defaultMarshaller.MarshalNamespace(
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
            this.ParseFeature(path, localContext.Context, localContext.Namespace, featureName);
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
                        throw new VowpalWabbitJsonException(this.reader, "Unxpected token " + reader.TokenType + " while deserializing dense feature array");
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
        private void ParseFeature(List<VowpalWabbitJsonParseContext> path, VowpalWabbitMarshalContext context, Namespace ns, string featureName)
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
                    this.WrapInNamespace(path, featureName, lastContext => this.ParseFeatureArray(path));
                    break;
                default:
                    throw new VowpalWabbitJsonException(this.reader, "Unexpected token " + reader.TokenType + " while deserializing primitive feature");
            }
        }

        /// <summary>
        /// Expects: "1,2.2,3]" (excluding the leading [)
        /// </summary>
        private void ParseFeatureArray(List<VowpalWabbitJsonParseContext> path)
        {
            var context = path.Last().Context;
            var ns = path.Last().Namespace;

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
                    case JsonToken.StartObject:
                        ParseProperties(path);
                        break;
                    case JsonToken.EndArray:
                        return;
                    case JsonToken.Null:
                        // just ignore nulls
                        break;
                    default:
                        throw new VowpalWabbitJsonException(this.reader, "Unxpected token " + reader.TokenType + " while deserializing dense feature array");
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

        private void WrapInNamespace(List<VowpalWabbitJsonParseContext> path, string namespaceValue, Action<VowpalWabbitJsonParseContext> action)
        {
            VowpalWabbitJsonParseContext parseContext = null;
            VowpalWabbitMarshalContext marshalContext = null;
            try
            {
                var ns = new Namespace(this.vw, namespaceValue);
                marshalContext = new VowpalWabbitMarshalContext(this.vw, this.DefaultNamespaceContext.ExampleBuilder);

                parseContext = new VowpalWabbitJsonParseContext
                {
                    Namespace = ns,
                    Context = marshalContext,
                    JsonProperty = namespaceValue
                };

                // the namespace is only added on dispose, to be able to check if at least a single feature has been added
                marshalContext.NamespaceBuilder = marshalContext.ExampleBuilder.AddNamespace(ns.FeatureGroup);

                var position = 0;
                var stringExample = marshalContext.StringExample;
                if (marshalContext.StringExample != null)
                    position = stringExample.Append(ns.NamespaceString).Length;

                path.Add(parseContext);

                action(parseContext);

                // append default namespaces features if we found some
                if (this.vw.Settings.EnableStringExampleGeneration)
                {
                    var str = marshalContext.ToString();
                    if (str.Length > 0)
                        this.namespaceStrings.Add(str);
                }

                this.featureCount += (int)marshalContext.NamespaceBuilder.FeatureCount;
            }
            finally
            {
                path.RemoveAt(path.Count - 1);

                if (marshalContext.NamespaceBuilder != null)
                {
                    marshalContext.NamespaceBuilder.Dispose();
                    marshalContext.NamespaceBuilder = null;
                }

                if (parseContext != null && parseContext.Context != null)
                {
                    parseContext.Context.Dispose();
                    parseContext.Context = null;
                }
            }
        }

        /// <summary>
        /// Parses { "feature1":1, "feature2":true, .... }
        /// </summary>
        private void ParseNamespaceAndFeatures(List<VowpalWabbitJsonParseContext> path, string namespaceValue)
        {
            this.WrapInNamespace(path, namespaceValue, context => this.ParseProperties(path));
        }

        private void ParseProperties(List<VowpalWabbitJsonParseContext> path)
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
                            throw new VowpalWabbitJsonException(this.reader, "Unexpected end while parsing namespace");

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
    }

    /// <summary>
    /// A parsing context holding the current state during JSON parsing.
    /// </summary>
    public sealed class VowpalWabbitJsonParseContext
    {
        /// <summary>
        /// The current marshalling context.
        /// </summary>
        public VowpalWabbitMarshalContext Context { get; set; }

        /// <summary>
        /// The current namespace.
        /// </summary>
        public Namespace Namespace { get; set; }

        /// <summary>
        /// The current JSON property being processed.
        /// </summary>
        public string JsonProperty { get; set; }
    }
}
