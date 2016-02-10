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
        public override void Learn()
        {
            this.vw.Learn(this.Example);
        }

        /// <summary>
        /// Learn from this example and returns the current prediction for it.
        /// </summary>
        /// <typeparam name="TPrediction">The prediction type.</typeparam>
        /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
        /// <returns>The prediction for the this example.</returns>
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
        /// <returns>The prediction for the this example.</returns>
        public override TPrediction Predict<TPrediction>(IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
            return this.vw.Predict<TPrediction>(this.Example, predictionFactory);
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
