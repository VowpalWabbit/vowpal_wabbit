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
        /// <summary>
        /// Base class for JSON deserialization result.
        /// </summary>
        public abstract class VowpalWabbitExamples : IDisposable
        {
            protected readonly VowpalWabbit vw;

            protected VowpalWabbitExamples(VowpalWabbit vw)
            {
                Contract.Requires(vw != null);
                this.vw = vw;
            }

            /// <summary>
            /// Learns from this example.
            /// </summary>
            public abstract void Learn();

            /// <summary>
            /// Predicts for this example.
            /// </summary>
            public abstract void Predict();

            /// <summary>
            /// Learn from this example and returns the current prediction for it.
            /// </summary>
            /// <typeparam name="TPrediction">The prediction type.</typeparam>
            /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
            /// <returns>The prediction for the given <paramref name="example"/>.</returns>
            public abstract TPrediction Learn<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory);

            /// <summary>
            /// Predicts for this example and returns the current prediction for it.
            /// </summary>
            /// <typeparam name="TPrediction">The prediction type.</typeparam>
            /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
            /// <returns>The prediction for the given <paramref name="example"/>.</returns>
            public abstract TPrediction Predict<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory);

            /// <summary>
            /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
            /// </summary>
            public abstract void Dispose();
        }

        /// <summary>
        /// Result for a single example.
        /// </summary>
        public sealed class SingleVowpalWabbitExample : VowpalWabbitExamples
        {
            /// <summary>
            /// Initializes a new instance of the <see cref="SingleVowpalWabbitExample"/> class.
            /// </summary>
            public SingleVowpalWabbitExample(VowpalWabbit vw, VowpalWabbitExample example)
                : base(vw)
            {
                this.Example = example;
            }

            /// <summary>
            /// Single example or the shared
            /// </summary>
            public VowpalWabbitExample Example { get; private set; }

            /// <summary>
            /// Learns from this example.
            /// </summary>
            public override void Learn()
            {
                this.vw.Learn(this.Example);
            }

            /// <summary>
            /// Learn from this example and returns the current prediction for it.
            /// </summary>
            /// <typeparam name="TPrediction">The prediction type.</typeparam>
            /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
            /// <returns>The prediction for the given <paramref name="example"/>.</returns>
            public override TPrediction Learn<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
            {
                return this.vw.Learn<TPrediction>(this.Example, predictionFactory);
            }

            /// <summary>
            /// Predicts for this example.
            /// </summary>
            public override void Predict()
            {
                this.vw.Predict(this.Example);
            }

            /// <summary>
            /// Predicts for this example and returns the current prediction for it.
            /// </summary>
            /// <typeparam name="TPrediction">The prediction type.</typeparam>
            /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
            /// <returns>The prediction for the given <paramref name="example"/>.</returns>
            public override TPrediction Predict<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
            {
                return this.vw.Predict<TPrediction>(this.Example, predictionFactory);
            }

            /// <summary>
            /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
            /// </summary>
            public override void Dispose()
            {
                this.Dispose(true);
                GC.SuppressFinalize(this);
            }

            private void Dispose(bool disposing)
            {
                if (disposing)
                {
                    if (this.Example != null)
                    {
                        this.Example.Dispose();
                        this.Example = null;
                    }
                }
            }
        }

        /// <summary>
        /// Result for multiline examples.
        /// </summary>
        public sealed class MultilineVowpalWabbitExamples : VowpalWabbitExamples
        {
            /// <summary>
            /// Initializes a new instance of the <see cref="MultilineVowpalWabbitExamples"/> class.
            /// </summary>
            public MultilineVowpalWabbitExamples(VowpalWabbit vw, VowpalWabbitExample shared, VowpalWabbitExample[] examples)
                : base(vw)
            {
                Contract.Requires(examples != null);

                this.SharedExample = shared;
                this.Examples = examples;
            }

            /// <summary>
            /// Single example or the shared
            /// </summary>
            public VowpalWabbitExample SharedExample { get; private set; }

            /// <summary>
            /// The multi-line examples
            /// </summary>
            public VowpalWabbitExample[] Examples { get; private set; }

            /// <summary>
            /// Calls learn or predict for the set of examples. Does required filtering of potential new line examples.
            /// </summary>
            private TPrediction Execute<TPrediction>(Action<VowpalWabbitExample> predictOrLearn, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory = null)
            {
                // firstExample will contain prediction result
                VowpalWabbitExample firstExample = null;
                VowpalWabbitExample empty = null;
                try
                {
                    if (this.SharedExample != null && !this.SharedExample.IsNewLine)
                    {
                        predictOrLearn(this.SharedExample);
                        firstExample = this.SharedExample;
                    }

                    foreach (var ex in this.Examples)
                    {
                        if (!ex.IsNewLine)
                        {
                            predictOrLearn(ex);

                            if (firstExample == null)
                                firstExample = ex;
                        }
                    }

                    // signal end-of-block
                    empty = vw.GetOrCreateEmptyExample();
                    predictOrLearn(empty);

                    return predictionFactory != null ? firstExample.GetPrediction(vw, predictionFactory) : default(TPrediction);
                }
                finally
                {
                    if (empty != null)
                        empty.Dispose();
                }
            }

            /// <summary>
            /// Learns from these examples.
            /// </summary>
            public override void Learn()
            {
                // unfortunately can't specify <void>
                this.Execute<int>(ex => this.vw.Learn(ex));
            }

            /// <summary>
            /// Learn from these examples and returns the current prediction for it.
            /// </summary>
            /// <typeparam name="TPrediction">The prediction type.</typeparam>
            /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
            /// <returns>The prediction for the given <paramref name="example"/>.</returns>
            public override TPrediction Learn<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
            {
                return this.Execute(ex => this.vw.Learn(ex), predictionFactory);
            }

            /// <summary>
            /// Predicts for these examples.
            /// </summary>
            public override void Predict()
            {
                // unfortunately can't specify <void>
                this.Execute<int>(ex => this.vw.Predict(ex));
            }

            /// <summary>
            /// Predicts for these examples and returns the current prediction for it.
            /// </summary>
            /// <typeparam name="TPrediction">The prediction type.</typeparam>
            /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
            /// <returns>The prediction for the given <paramref name="example"/>.</returns>
            public override TPrediction Predict<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
            {
                return this.Execute(ex => this.vw.Predict(ex), predictionFactory);
            }

            /// <summary>
            /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
            /// </summary>
            public override void Dispose()
            {
                this.Dispose(true);
                GC.SuppressFinalize(this);
            }

            private void Dispose(bool disposing)
            {
                if (disposing)
                {
                    if (this.SharedExample != null)
                    {
                        this.SharedExample.Dispose();
                        this.SharedExample = null;
                    }

                    if (this.Examples != null)
                    {
                        foreach (var ex in this.Examples)
                            ex.Dispose();

                        this.Examples = null;
                    }
                }
            }
        }

        private readonly VowpalWabbit vw;
        private readonly VowpalWabbitDefaultMarshaller defaultMarshaller;
        private readonly JsonSerializer jsonSerializer;

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
        /// Parses and creates the example.
        /// </summary>
        /// <param name="json">The example to parse.</param>
        /// <param name="label">
        /// Optional label, taking precedence over "_label" property found in <paramref name="json"/>.
        /// If null, <paramref name="json"/> will be inspected and the "_label" property used as label.
        /// </param>
        /// <returns>The VowpalWabbit native example.</returns>
        public VowpalWabbitExamples ParseAndCreate(string json, ILabel label = null, int? index = null)
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
        /// <returns>The VowpalWabbit native example.</returns>
        public VowpalWabbitExamples ParseAndCreate(JsonReader reader, ILabel label = null, int? index = null)
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
        public void Parse(string json, ILabel label = null, int? index = null)
        {
            using (var textReader = new JsonTextReader(new StringReader(json)))
            {
                this.Parse(textReader, label);
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
                        switch (reader.TokenType)
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

                    throw new VowpalWabbitJsonException(reader.Path, "Unexpected end");
                });
        }

        /// <summary>
        /// Creates the examples ready for learning or prediction.
        /// </summary>
        public VowpalWabbitExamples CreateExamples()
        {
            try
            {
                if (this.ExampleBuilders == null)
                {
                    return new SingleVowpalWabbitExample(this.vw, this.ExampleBuilder.CreateExample());
                }
                else
                {
                    // making sure we don't leak memory
                    VowpalWabbitExample sharedExample = null;
                    var examples = new VowpalWabbitExample[this.ExampleBuilders.Count];

                    try
                    {
                        // mark shared example as shared
                        this.defaultMarshaller.MarshalLabel(this.ExampleBuilder.Context, SharedLabel.Instance);

                        sharedExample = this.ExampleBuilder.CreateExample();
                        for (int i = 0; i < this.ExampleBuilders.Count; i++)
			                examples[i] = this.ExampleBuilders[i].CreateExample();

                        return new MultilineVowpalWabbitExamples(this.vw, sharedExample, examples);
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
