// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitJsonThreadedPrediction.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW
{
    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbitJson"/> instances.
    /// </summary>
    public sealed class VowpalWabbitJsonThreadedPrediction : VowpalWabbitThreadedPredictionBase<VowpalWabbitJson>
    {
        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction{TExample}"/>.
        /// </summary>
        /// <param name="model">The model used by each pool instance.</param>
        public VowpalWabbitJsonThreadedPrediction(VowpalWabbitModel model = null)
            : base(model)
        {
        }

        /// <summary>
        /// Creates a new instance of <see cref="VowpalWabbit{TExample}"/>.
        /// </summary>
        /// <param name="vw">The wrapped vw instance.</param>
        protected override VowpalWabbitJson InternalCreate(VowpalWabbit vw)
        {
            return new VowpalWabbitJson(vw);
        }
    }
}
