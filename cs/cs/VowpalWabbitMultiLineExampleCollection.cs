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
using System.Text;
using System.Threading.Tasks;

namespace VW
{
    /// <summary>
    /// Result for multiline examples.
    /// </summary>
    public sealed class VowpalWabbitMultiLineExampleCollection : VowpalWabbitExampleCollection
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitMultiLineExampleCollection"/> class.
        /// </summary>
        public VowpalWabbitMultiLineExampleCollection(VowpalWabbit vw, VowpalWabbitExample shared, VowpalWabbitExample[] examples)
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
            Contract.Requires(predictOrLearn != null);

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
        /// <returns>The prediction for the this example.</returns>
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
        /// <returns>The prediction for the this example.</returns>
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
}
