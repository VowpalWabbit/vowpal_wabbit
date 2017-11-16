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
using System.Threading.Tasks;
using VW.Labels;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// The current JSON parse state.
    /// </summary>
    public sealed class VowpalWabbitJsonParseState
    {
        /// <summary>
        /// The native VW instance.
        /// </summary>
        public VowpalWabbit VW { get; set; }

        /// <summary>
        /// The JSON reader.
        /// </summary>
        public JsonReader Reader { get; set; }

        /// <summary>
        /// The VW example JSON builder.
        /// </summary>
        public VowpalWabbitJsonBuilder JsonBuilder { get; set; }

        /// <summary>
        /// The current property path within the JSON.
        /// </summary>
        public List<VowpalWabbitJsonParseContext> Path { get; set; }

        /// <summary>
        /// The current _multi element index.
        /// </summary>
        public int MultiIndex { get; set; }

        /// <summary>
        /// Triggers parsing at the current state of the <see cref="Reader"/> using the default namespace.
        /// </summary>
        public void Parse()
        {
            using (var context = new VowpalWabbitMarshalContext(this.VW, this.JsonBuilder.DefaultNamespaceContext.ExampleBuilder))
            {
                var ns = new Namespace(this.VW);
                this.Parse(context, ns);
            }
        }

        /// <summary>
        /// Triggers parsing at the current state of the <see cref="Reader"/> using the given <paramref name="namespaceContext"/>.
        /// </summary>
        /// <param name="namespaceContext">The namespace the JSON should be marshalled into.</param>
        /// <param name="ns">The namespace the JSON should be marshalled into.</param>
        public void Parse(VowpalWabbitMarshalContext namespaceContext, Namespace ns)
        {
            this.JsonBuilder.Parse(this.Path, namespaceContext, ns);
        }
    }

    /// <summary>
    /// Delegate definition for JSON parsing extension. E.g. if one wants to extract "_timestamp" or a like.
    /// </summary>
    /// <param name="state">The current parsing state.</param>
    /// <param name="property">The property encountered.</param>
    /// <returns>True if the extension handled this property, false otherwise.</returns>
    /// <remarks>Only fires for "ignore prefixed" properties.</remarks>
    public delegate bool VowpalWabbitJsonExtension(VowpalWabbitJsonParseState state, string property);

    /// <summary>
    /// A deserializer from JSON to Vowpal Wabbit native examples.
    /// </summary>
    public sealed class VowpalWabbitJsonSerializer : IDisposable
    {
        private readonly IVowpalWabbitExamplePool vwPool;
        private readonly JsonSerializer jsonSerializer;
        private readonly VowpalWabbitJsonReferenceResolver referenceResolver;

        private int unresolved;
        private readonly object lockObject = new object();
        private bool ready = false;
        private List<Action> marshalRequests;
        private List<VowpalWabbitJsonExtension> extensions;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJson"/> class.
        /// </summary>
        /// <param name="vwPool">The VW native instance.</param>
        /// <param name="referenceResolver">An optional reference resolver.</param>
        public VowpalWabbitJsonSerializer(IVowpalWabbitExamplePool vwPool, VowpalWabbitJsonReferenceResolver referenceResolver = null)
        {
            Contract.Requires(vwPool != null);

            this.extensions = new List<VowpalWabbitJsonExtension> { this.HandleMultiProperty };
            this.jsonSerializer = new JsonSerializer();

            this.vwPool = vwPool;
            this.referenceResolver = referenceResolver;

            this.ExampleBuilder = new VowpalWabbitJsonBuilder(this, this.vwPool, VowpalWabbitDefaultMarshaller.Instance, this.jsonSerializer);
        }

        /// <summary>
        /// Registers a parsing extension.
        /// </summary>
        /// <param name="extension">The extension to be rgistered.</param>
        public void RegisterExtension(VowpalWabbitJsonExtension extension)
        {
            this.extensions.Add(extension);
        }

        /// <summary>
        /// Userful if this deserializer is published through VowpalWabbitJsonReferenceResolver.
        /// </summary>
        public object UserContext { get; set; }

        /// <summary>
        /// Single line example or shared example.
        /// </summary>
        public VowpalWabbitJsonBuilder ExampleBuilder { get; private set; }

        /// <summary>
        /// Multi-line examples.
        /// </summary>
        public List<VowpalWabbitJsonBuilder> ExampleBuilders { get; private set; }

        internal VowpalWabbitJsonReferenceResolver ReferenceResolver
        {
            get { return this.referenceResolver; }
        }

        internal void IncreaseUnresolved()
        {
            // only called during the initial parsing run
            this.unresolved++;
        }

        internal bool Resolve(Action marshal)
        {
            lock (this.lockObject)
            {
                // ready is false until the initial parsing run is complete
                if (this.ready)
                {
                    // the object doesn't get anymore unresolved marshal requests
                    if (this.marshalRequests != null)
                    {
                        foreach (var req in this.marshalRequests)
                            req();

                        this.unresolved -= this.marshalRequests.Count;

                        this.marshalRequests = null;
                    }

                    marshal();
                    this.unresolved--;

                    if (this.unresolved < 0)
                        throw new InvalidOperationException("Number of unresolved requested must not be negative");

                    return this.unresolved == 0;
                }
                else
                {
                    // we need to track the requests and wait until the initial parsing is done
                    if (this.marshalRequests == null)
                        this.marshalRequests = new List<Action>();

                    this.marshalRequests.Add(marshal);

                    return false;
                }
            }
        }

        /// <summary>
        /// Creates the VW example, be it single or multi-line.
        /// </summary>
        /// <returns>The marshalled VW example.</returns>
        public VowpalWabbitExampleCollection CreateExamples()
        {
            lock (this.lockObject)
            {
                if (this.unresolved == 0)
                    return this.CreateExamplesInternal();

                if (this.marshalRequests != null && this.unresolved == this.marshalRequests.Count)
                {
                    return this.CreateExamplesInternal();
                }

                // wait for delayed completion
                this.ready = true;

                return null;
            }
        }

        /// <summary>
        /// Creates the VW example, be it single or multi-line.
        /// </summary>
        /// <param name="label">The label to be applied.</param>
        /// <param name="index">The index of the example in the multi-line example this label should be applied on.</param>
        /// <returns></returns>
        public VowpalWabbitExampleCollection CreateExamples(ILabel label, int index)
        {
            if (index >= this.ExampleBuilders.Count)
                throw new InvalidDataException($"Label index {index} is invalid. Only {this.ExampleBuilders.Count} examples available.");

            VowpalWabbitDefaultMarshaller.Instance.MarshalLabel(
                this.ExampleBuilders[index].DefaultNamespaceContext,
                label);

            return this.CreateExamples();
        }

        /// <summary>
        /// Parses and creates the example.
        /// </summary>
        /// <param name="json">The example to parse.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <param name="index">Optional index of example the given label should be applied for multi-line examples.</param>
        /// <returns>The VowpalWabbit native example.</returns>
        public VowpalWabbitExampleCollection ParseAndCreate(string json, ILabel label = null, int? index = null)
        {
            this.Parse(json, label, index);
            return this.CreateExamples();
        }

        /// <summary>
        /// Parses the example.
        /// </summary>
        /// <param name="reader">The example to parse.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="reader"/>.
        /// If null, <paramref name="reader"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <param name="index">Optional index of example the given label should be applied for multi-line examples.</param>
        /// <returns>The VowpalWabbit native example.</returns>
        public VowpalWabbitExampleCollection ParseAndCreate(JsonReader reader, ILabel label = null, int? index = null)
        {
            this.Parse(reader, label, index);
            return this.CreateExamples();
        }

        /// <summary>
        /// Parses the example.
        /// </summary>
        /// <param name="json">The example to parse.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <param name="index">Optional index of example the given label should be applied for multi-line examples.</param>
        public void Parse(string json, ILabel label = null, int? index = null)
        {
            using (var textReader = new JsonTextReader(new StringReader(json)))
            {
                this.Parse(textReader, label);
            }
        }

        /// <summary>
        /// Returns the number of action dependent examples found within <paramref name="json"/>.
        /// </summary>
        /// <param name="json">The JSON to be inspected.</param>
        /// <returns>Returns the number of action dependent examples.</returns>
        public static int GetNumberOfActionDependentExamples(string json)
        {
            using (var textReader = new JsonTextReader(new StringReader(json)))
            {
                return GetNumberOfActionDependentExamples(textReader);
            }
        }

        /// <summary>
        /// Returns the number of action dependent examples found within <paramref name="reader"/>.
        /// </summary>
        /// <param name="reader">The JSON.</param>
        /// <param name="multiProperty">The optional multi property name.</param>
        /// <returns>Returns the number of action dependent examples.</returns>
        public static int GetNumberOfActionDependentExamples(JsonReader reader, string multiProperty = PropertyConfiguration.MultiPropertyDefault)
        {
            // handle the case when the reader is already positioned at JsonToken.StartObject
            if (reader.TokenType == JsonToken.None && !reader.Read())
                throw new VowpalWabbitJsonException(reader, "Expected non-empty JSON");

            if (reader.TokenType != JsonToken.StartObject)
                throw new VowpalWabbitJsonException(reader, "Expected start object");

            while (reader.Read())
            {
                if (!(reader.TokenType == JsonToken.PropertyName && (string)reader.Value == multiProperty))
                {
                    reader.Skip();
                    continue;
                }

                if (!reader.Read() || reader.TokenType != JsonToken.StartArray)
                    throw new VowpalWabbitJsonException(reader, "Expected start arrray");

                var exampleCount = 0;
                while (reader.Read() && reader.TokenType != JsonToken.EndArray)
                {
                    exampleCount++;
                    reader.Skip();
                }

                return exampleCount;
            }

            return 0;
        }

        private bool HandleMultiProperty(VowpalWabbitJsonParseState state, string property)
        {
            var multiPropertyName = this.vwPool.Native.Settings.PropertyConfiguration.MultiProperty;
            if (!property.Equals(multiPropertyName, StringComparison.OrdinalIgnoreCase))
                return false;

            var reader = state.Reader;
            if (!reader.Read() || reader.TokenType != JsonToken.StartArray)
                throw new VowpalWabbitJsonException(reader, "Expected start array for '" + multiPropertyName + "'");

            if (this.ExampleBuilders == null)
                this.ExampleBuilders = new List<VowpalWabbitJsonBuilder>();

            state.MultiIndex = 0;

            while (reader.Read())
            {
                switch (reader.TokenType)
                {
                    case JsonToken.StartObject:
                        VowpalWabbitJsonBuilder builder = null;

                        try
                        {
                            builder = new VowpalWabbitJsonBuilder(this, this.vwPool, VowpalWabbitDefaultMarshaller.Instance, this.jsonSerializer, state.MultiIndex);
                            this.ExampleBuilders.Add(builder);
                        }
                        catch (Exception)
                        {
                            builder.Dispose();
                            throw;
                        }

                        // pass the label to the selected example
                        builder.Parse(reader, index != null && index == this.ExampleBuilders.Count - 1 ? label : null, this.extensions);

                        state.MultiIndex++;
                        break;
                    case JsonToken.EndArray:
                        return true;
                    default:
                        throw new VowpalWabbitJsonException(reader, "Unexpected token: " + reader.TokenType);
                }
            }

            throw new VowpalWabbitJsonException(reader, "Unexpected end");
        }

        // TODO: keeping it local might be nicer...
        private int? index;
        private ILabel label;

        /// <summary>
        /// Parses the example.
        /// </summary>
        /// <param name="reader">The example to parse.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="reader"/>.
        /// If null, <paramref name="reader"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <param name="index">Optional index of example the given label should be applied for multi-line examples.</param>
        public void Parse(JsonReader reader, ILabel label = null, int? index = null)
        {
            this.index = index;
            this.label = label;

            // only pass the label if it's not targeted at a particular index
            this.ExampleBuilder.Parse(reader, index == null ? label : null, this.extensions);

            // check if the outer example found a label
            if (this.ExampleBuilder.Label != null)
            {
                if (this.ExampleBuilder.LabelIndex >= this.ExampleBuilders.Count)
                    throw new InvalidDataException($"Label index {this.ExampleBuilder.LabelIndex} is invalid. Only {this.ExampleBuilders.Count} examples available.");

                VowpalWabbitDefaultMarshaller.Instance.MarshalLabel(
                    this.ExampleBuilders[this.ExampleBuilder.LabelIndex].DefaultNamespaceContext,
                    this.ExampleBuilder.Label);
            }
        }

        /// <summary>
        /// Creates the examples ready for learning or prediction.
        /// </summary>
        public VowpalWabbitExampleCollection CreateExamplesInternal()
        {
            try
            {
                if (this.ExampleBuilders == null)
                {
                    return new VowpalWabbitSingleLineExampleCollection(this.vwPool.Native, this.ExampleBuilder.CreateExample());
                }
                else
                {
                    // making sure we don't leak memory
                    VowpalWabbitExample sharedExample = null;
                    var examples = new VowpalWabbitExample[this.ExampleBuilders.Count];

                    try
                    {
                        // mark shared example as shared
                        VowpalWabbitDefaultMarshaller.Instance.MarshalLabel(this.ExampleBuilder.DefaultNamespaceContext, SharedLabel.Instance);

                        sharedExample = this.ExampleBuilder.CreateExample();
                        for (int i = 0; i < this.ExampleBuilders.Count; i++)
                            examples[i] = this.ExampleBuilders[i].CreateExample();

                        return new VowpalWabbitMultiLineExampleCollection(this.vwPool.Native, sharedExample, examples);
                    }
                    catch (Exception)
                    {
                        if (sharedExample != null)
                            sharedExample.Dispose();

                        foreach (var e in examples)
                            if (e != null)
                                e.Dispose();

                        throw;
                    }
                }
            }
            finally
            {
                this.ExampleBuilder.Dispose();
                this.ExampleBuilder = null;

                if (this.ExampleBuilders != null)
                {
                    foreach (var eb in this.ExampleBuilders)
                        eb.Dispose();

                    this.ExampleBuilders = null;
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
            // Remark: might be called multiple times from VowpalWabbitJsonReferenceResolver
            if (disposing)
            {
                // cleanup in case CreateExample() wasn't called
                if (this.ExampleBuilder != null)
                {
                    this.ExampleBuilder.Dispose();
                    this.ExampleBuilder = null;
                }

                if (this.ExampleBuilders != null)
                {
                    foreach (var eb in this.ExampleBuilders)
                        eb.Dispose();

                    this.ExampleBuilders = null;
                }
            }
        }
    }
}
