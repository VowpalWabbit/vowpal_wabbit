// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSingleLineExampleCollection.cs">
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
using VW.Labels;

namespace VW
{
    /// <summary>
    /// Result for a single example.
    /// </summary>
    public sealed class VowpalWabbitSingleLineExampleCollection : VowpalWabbitExampleCollection
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitSingleLineExampleCollection"/> class.
        /// </summary>
        public VowpalWabbitSingleLineExampleCollection(VowpalWabbit vw, VowpalWabbitExample example)
            : base(vw)
        {
            Contract.Requires(example != null);

            this.Example = example;
        }

        /// <summary>
        /// Single example or the shared
        /// </summary>
        public VowpalWabbitExample Example { get; private set; }

        /// <summary>
        /// Learns from this example.
        /// </summary>
        protected override void LearnInternal(VowpalWabbit vw)
        {
            vw.Learn(this.Example);
        }

        /// <summary>
        /// Learn from this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="vw">The VW native instance.</param>
        /// <returns>The prediction for the this example.</returns>
        protected override TPrediction LearnInternal<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw)
        {
            return vw.Learn<TPrediction>(this.Example, predictionFactory);
        }

        /// <summary>
        /// Predicts for this example.
        /// </summary>
        protected override void PredictInternal(VowpalWabbit vw)
        {
            vw.Predict(this.Example);
        }

        /// <summary>
        /// Predicts for this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <param name="vw">The VW instance that should be used for prediction.</param>
        /// <returns>The prediction for the this example.</returns>
        protected override TPrediction PredictInternal<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, VowpalWabbit vw)
        {
            return vw.Predict<TPrediction>(this.Example, predictionFactory);
        }

        /// <summary>
        /// The optional string version of the example.
        /// </summary>
        public override string VowpalWabbitString
        {
            get
            {
                return this.Example.VowpalWabbitString;
            }
        }

        /// <summary>
        /// The number of feature this example holds.
        /// </summary>
        public override ulong NumberOfFeatures
        {
            get { return this.Example.NumberOfFeatures; }
        }

        /// <summary>
        /// All labels this example holds.
        /// </summary>
        public override IEnumerable<ILabel> Labels
        {
            get
            {
                yield return this.Example.Label;
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
                if (this.Example != null)
                {
                    this.Example.Dispose();
                    this.Example = null;
                }
            }
        }
    }
}
