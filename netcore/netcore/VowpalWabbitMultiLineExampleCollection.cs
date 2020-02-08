// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitMultiLineExampleCollection.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using VW.Labels;

namespace VW
{
    /// <summary>
    /// Result for multiline examples.
    /// </summary>
    public sealed class VowpalWabbitMultiLineExampleCollection : VowpalWabbitExampleCollection
    {
        private readonly ulong numberOfFeatures;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitMultiLineExampleCollection"/> class.
        /// </summary>
        public VowpalWabbitMultiLineExampleCollection(VowpalWabbit vw, VowpalWabbitExample shared, VowpalWabbitExample[] examples)
            : base(vw)
        {
            Contract.Requires(examples != null);

            this.SharedExample = shared;
            this.Examples = examples;

            if (shared != null)
                numberOfFeatures += shared.NumberOfFeatures;

            foreach (var e in examples)
                if (e != null)
                    numberOfFeatures += e.NumberOfFeatures;
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
        /// The number of feature this example holds.
        /// </summary>
        public override ulong NumberOfFeatures
        {
            get { return this.numberOfFeatures; }
        }

        /// <summary>
        /// Calls learn or predict for the set of examples. Does required filtering of potential new line examples.
        /// </summary>
        private TPrediction Execute<TPrediction>(VowpalWabbit vw, Action<List<VowpalWabbitExample>> predictOrLearn, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory = null)
        {
            Contract.Requires(predictOrLearn != null);

            // firstExample will contain prediction result
            VowpalWabbitExample firstExample = null;
            VowpalWabbitExample empty = null;
            try
            {
                var ecCol = new List<VowpalWabbitExample>();

                if (this.SharedExample != null && !this.SharedExample.IsNewLine)
                {
                    firstExample = this.SharedExample;
                    ecCol.Add(firstExample);
                }

                foreach (var ex in this.Examples)
                {
                    if (!ex.IsNewLine)
                    {
                        ecCol.Add(ex);

                        if (firstExample == null)
                            firstExample = ex;
                    }
                }

                // signal end-of-block
                empty = vw.GetOrCreateNativeExample();
                empty.MakeEmpty(vw);

                predictOrLearn(ecCol);

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
        protected override void LearnInternal(VowpalWabbit vw)
        {
            // unfortunately can't specify <void>
            this.Execute<int>(vw, ex => vw.Learn(ex));
        }

        /// <summary>
        /// Learn from these examples and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="vw">The VW instance that should be used for learning.</param>
        /// <returns>The prediction for the this example.</returns>
        protected override TPrediction LearnInternal<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw)
        {
            return this.Execute(vw, ex => vw.Learn(ex), predictionFactory);
        }

        /// <summary>
        /// Predicts for these examples.
        /// </summary>
        protected override void PredictInternal(VowpalWabbit vw)
        {
            // unfortunately can't specify <void>
            this.Execute<int>(vw, ex => vw.Predict(ex));
        }

        /// <summary>
        /// Predicts for these examples and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="vw">The native VW instance.</param>
        /// <returns>The prediction for the this example.</returns>
        protected override TPrediction PredictInternal<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw)
        {
            return this.Execute(vw, ex => vw.Predict(ex), predictionFactory);
        }

        /// <summary>
        /// The optional string version of the example.
        /// </summary>
        public override string VowpalWabbitString
        {
            get
            {
                var str = new List<string>();

                if (this.SharedExample != null)
                    str.Add(this.SharedExample.VowpalWabbitString);

                str.AddRange(this.Examples.Select(e => e.VowpalWabbitString));

                // filter empty example
                return string.Join("\n", str.Where(s => !string.IsNullOrWhiteSpace(s)));
            }
        }

        /// <summary>
        /// All labels this example holds.
        /// </summary>
        public override IEnumerable<ILabel> Labels
        {
            get
            {
                return this.Examples.Select(e => e.Label);
            }
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
                        if (ex != null)
                            ex.Dispose();

                    this.Examples = null;
                }
            }
        }
    }
}
