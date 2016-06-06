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
using VW.Labels;
using VW.Labels;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// A deserializer from JSON to Vowpal Wabbit native examples.
    /// </summary>
    public sealed class VowpalWabbitJsonSerializer : IDisposable
    {
        private readonly IVowpalWabbitExamplePool vwPool;
        private readonly VowpalWabbitDefaultMarshaller defaultMarshaller;
        private readonly JsonSerializer jsonSerializer;
        private readonly VowpalWabbitJsonReferenceResolver referenceResolver;

        private int unresolved;
        private readonly object lockObject = new object();
        private bool ready = false;
        private List<Action> marshalRequests;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitJson"/> class.
        /// </summary>
        /// <param name="vw">The VW native instance.</param>
        public VowpalWabbitJsonSerializer(IVowpalWabbitExamplePool vwPool, VowpalWabbitJsonReferenceResolver referenceResolver = null)
        {
            Contract.Requires(vwPool != null);

            this.vwPool = vwPool;
            this.referenceResolver = referenceResolver;
            this.defaultMarshaller = VowpalWabbitDefaultMarshaller.Instance;
            this.jsonSerializer = new JsonSerializer();

            this.ExampleBuilder = new VowpalWabbitJsonBuilder(this, this.vwPool, this.defaultMarshaller, this.jsonSerializer);
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

        public static int GetNumberOfActionDependentExamples(string json)
        {
            using (var textReader = new JsonTextReader(new StringReader(json)))
            {
                return GetNumberOfActionDependentExamples(textReader);
            }
        }

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
                while(reader.Read() && reader.TokenType != JsonToken.EndArray)
                {
                    exampleCount++;
                    reader.Skip();
                }

                return exampleCount;
            }

            return 0;
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
        public void Parse(JsonReader reader, ILabel label = null, int? index = null)
        {
            var multiPropertyName = this.vwPool.Native.Settings.PropertyConfiguration.MultiProperty;

            // only pass the label if it's not targeted at a particular index
            this.ExampleBuilder.Parse(reader, index == null ? label : null,
                propertyName =>
                {
                    if (propertyName != multiPropertyName)
                        return false;

                    if (!reader.Read() || reader.TokenType != JsonToken.StartArray)
                        throw new VowpalWabbitJsonException(reader, "Expected start array for '" + multiPropertyName + "'");

                    if (this.ExampleBuilders == null)
                        this.ExampleBuilders = new List<VowpalWabbitJsonBuilder>();

                    while (reader.Read())
                    {
                        switch (reader.TokenType)
                        {
                            case JsonToken.StartObject:
                                VowpalWabbitJsonBuilder builder = null;

                                try
                                {
                                    builder = new VowpalWabbitJsonBuilder(this, this.vwPool, this.defaultMarshaller, this.jsonSerializer);
                                    this.ExampleBuilders.Add(builder);
                                }
                                catch (Exception)
                                {
                                    builder.Dispose();
                                    throw;
                                }

                                // pass the label to the selected example
                                builder.Parse(reader, index != null && index == this.ExampleBuilders.Count - 1 ? label : null);
                                break;
                            case JsonToken.EndArray:
                                return true;
                            default:
                                throw new VowpalWabbitJsonException(reader, "Unexpected token: " + reader.TokenType);
                        }
                    }

                    throw new VowpalWabbitJsonException(reader, "Unexpected end");
                });

            // check if the outer example foudn a label
            if (this.ExampleBuilder.Label != null)
            {
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
                        this.defaultMarshaller.MarshalLabel(this.ExampleBuilder.DefaultNamespaceContext, SharedLabel.Instance);

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
