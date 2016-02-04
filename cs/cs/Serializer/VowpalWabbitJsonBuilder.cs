using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    public sealed class VowpalWabbitJsonBuilder : IDisposable
    {
        private readonly VowpalWabbit vw;
        private readonly VowpalWabbitDefaultMarshaller defaultMarshaller;
        private readonly JsonSerializer jsonSerializer;
        private JsonReader reader;

        public VowpalWabbitJsonBuilder(VowpalWabbit vw, VowpalWabbitDefaultMarshaller defaultMarshaller, JsonSerializer jsonSerializer)
        {
            Contract.Requires(vw != null);
            Contract.Requires(defaultMarshaller != null);
            Contract.Requires(jsonSerializer != null);

            this.vw = vw;
            this.defaultMarshaller = new VowpalWabbitDefaultMarshaller();
            this.jsonSerializer = new JsonSerializer();

            this.Context = new VowpalWabbitMarshalContext(this.vw);
            this.DefaultNamespaceContext = new VowpalWabbitMarshalContext(this.vw, this.Context.ExampleBuilder);
        }

        /// <summary>
        /// The marshalling context. Can be modified until <see cref="CreateExample"/>.
        /// </summary>
        public VowpalWabbitMarshalContext Context { get; private set; }

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
                var vwExample = this.Context.ExampleBuilder.CreateExample();

                if (this.vw.Settings.EnableStringExampleGeneration)
                    vwExample.VowpalWabbitString = this.Context.StringExample.ToString();

                return vwExample;
            }
            finally
            {
                this.DefaultNamespaceContext.Dispose();
                this.DefaultNamespaceContext = null;
                this.Context.Dispose();
                this.Context = null;
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
        /// <returns>The VowpalWabbit native example.</returns>
        public void Parse(JsonReader reader, ILabel label = null, Func<string, bool> specialPropertyAction = null)
        {
            // avoid parameter passing for the sake of non-reentrantness
            this.reader = reader;

            if (label != null)
                this.defaultMarshaller.MarshalLabel(this.Context, label);

            // handle the case when the reader is already positioned at JsonToken.StartObject
            if (reader.TokenType == JsonToken.None && !reader.Read())
                return;

            if (reader.TokenType != JsonToken.StartObject)
                throw new VowpalWabbitJsonException(reader.Path, "Expected start object");

            Namespace defaultNamespace = new Namespace(this.vw);
            using (this.DefaultNamespaceContext.NamespaceBuilder = this.DefaultNamespaceContext.ExampleBuilder.AddNamespace(VowpalWabbitConstants.DefaultNamespace))
            {
                while (reader.Read())
                {
                    switch (reader.TokenType)
                    {
                        case JsonToken.PropertyName:
                            var propertyName = (string)reader.Value;
                            if (propertyName.StartsWith(FeatureIgnorePrefix))
                            {
                                // special fields
                                if (propertyName == "_label")
                                {
                                    // passed in label has precedence
                                    if (label == null)
                                        this.ParseLabel();
                                    else
                                        reader.Skip();
                                }
                                else
                                {
                                    // forward to handler
                                    if (specialPropertyAction == null || !specialPropertyAction(propertyName))
                                        reader.Skip(); // if not handled, skip it
                                }
                            }
                            else
                            {
                                if (!reader.Read())
                                    throw new VowpalWabbitJsonException(reader.Path, "Unexpected end");

                                if (reader.TokenType == JsonToken.StartObject)
                                    this.ParseNamespaceAndFeatures(this.Context, propertyName);
                                else
                                    this.ParseFeature(this.DefaultNamespaceContext, defaultNamespace, propertyName);

                            }
                            break;
                        case JsonToken.EndObject:
                            goto done;
                    }
                }
            done:

                // append default namespaces features if we found some
                if (this.DefaultNamespaceContext.StringExample != null && this.DefaultNamespaceContext.StringExample.Length > 0)
                {
                    this.Context.StringExample.AppendFormat(CultureInfo.InvariantCulture,
                        " | {0}", this.DefaultNamespaceContext.StringExample);
                }
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

                        this.defaultMarshaller.MarshalLabel(this.Context, label);
                    }
                    break;
                case JsonToken.Integer:
                case JsonToken.Float:
                case JsonToken.String:
                    {
                        // pass label directly to VW
                        var labelString = reader.Value.ToString();

                        this.Context.ExampleBuilder.ParseLabel(labelString);
                        // prefix with label
                        this.Context.AppendStringExample(false, "{0}", labelString);
                    }
                    break;
                default:
                    throw new VowpalWabbitJsonException(reader.Path, "Expected label object");
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
                        this.defaultMarshaller.MarshalFeatureStringEscape(context, ns, feature, (string)reader.Value);
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
                    throw new VowpalWabbitJsonException(reader.Path, "Unexpected token " + reader.TokenType + " while deserializing primitive feature");
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
                                throw new VowpalWabbitJsonException(reader.Path, "Unexpected end while parsing namespace");

                            this.ParseFeature(context, ns, featureName);
                            break;
                        case JsonToken.EndObject:
                            return;
                    }
                }
            });
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
                    this.DefaultNamespaceContext.Dispose();
                    this.DefaultNamespaceContext = null;
                }

                if (this.Context != null)
                {
                    this.Context.Dispose();
                    this.Context = null;
                }
            }
        }
    }
}
