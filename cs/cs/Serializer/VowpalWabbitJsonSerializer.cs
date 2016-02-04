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
    public sealed class VowpalWabbitJsonSerializer : IDisposable
    {
        internal const string FeatureIgnorePrefix = "_";

        public interface IResult
        {
        }

        public sealed class SingleResult : IResult
        {
            /// <summary>
            /// Single example or the shared
            /// </summary>
            public VowpalWabbitExample Example { get; set; }
        }

        public sealed class MultilineResult : IResult
        {
            /// <summary>
            /// Single example or the shared
            /// </summary>
            public VowpalWabbitExample Example { get; set; }

            /// <summary>
            /// The multi-line examples
            /// </summary>
            public List<VowpalWabbitExample> Examples { get; set; }
        }


        /// <summary>
        /// Mapping from properties to types for labels.
        /// </summary>
        private static readonly Dictionary<string, Type> labelPropertyMapping;

        private readonly VowpalWabbit vw;
        private readonly VowpalWabbitDefaultMarshaller defaultMarshaller;
        private readonly JsonSerializer jsonSerializer;

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

            this.ExampleBuilder = new VowpalWabbitJsonBuilder(vw, this.defaultMarshaller, this.jsonSerializer);
        }

        /// <summary>
        /// Single line example or shared example.
        /// </summary>
        public VowpalWabbitJsonBuilder ExampleBuilder { get; private set; }

        /// <summary>
        /// Multi-line examples.
        /// </summary>
        public List<VowpalWabbitJsonBuilder> ExampleBuilders { get; private set; }

        /// <summary>
        /// Parses the example.
        /// </summary>
        /// <param name="json">The example to parse.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>
        public void Parse(string json, ILabel label = null, int? index = null)
        {
            using (var textReader = new JsonTextReader(new StringReader(json)))
            {
                this.Parse(textReader, label);
            }
        }

        public void Parse(JsonReader reader, ILabel label = null, int? index = null)
        {
            // only pass the label if it's not targeted at a particular index
            this.ExampleBuilder.Parse(reader, index == null ? label : null,
                propertyName =>
                {
                    if (propertyName != "_multi")
                        return false;

                    if (!reader.Read() || reader.TokenType != JsonToken.StartArray)
                        throw new VowpalWabbitJsonException(reader.Path, "Expected start array for _multi");

                    if (this.ExampleBuilders == null)
                        this.ExampleBuilders = new List<VowpalWabbitJsonBuilder>();

                    while (reader.Read())
                    {
                        switch(reader.TokenType)
                        {
                            case JsonToken.StartObject:
                                VowpalWabbitJsonBuilder builder = null;

                                try
                                {
                                    builder = new VowpalWabbitJsonBuilder(this.vw, this.defaultMarshaller, this.jsonSerializer);
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
                                throw new VowpalWabbitJsonException(reader.Path, "Unexpected token: " + reader.TokenType);
                        }
                    }
                });

            // if this is a multi-line example, set the first example to be a shared example
            if (this.ExampleBuilders != null)
                this.defaultMarshaller.MarshalLabel(this.ExampleBuilder.Context, SharedLabel.Instance);
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
