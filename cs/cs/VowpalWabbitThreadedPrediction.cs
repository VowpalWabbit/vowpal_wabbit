// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitThreadedPrediction.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace VW
{
    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbit"/> instances.
    /// </summary>
    public sealed class VowpalWabbitThreadedPrediction : VowpalWabbitThreadedPredictionBase<VowpalWabbit>
    {
        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction"/>.
        /// </summary>
        /// <remarks>Decision Service client library needs default constructor.</remarks>
        public VowpalWabbitThreadedPrediction()
        {
        }

        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction"/>.
        /// </summary>
        /// <param name="model">The model used by each pool instance.</param>
        public VowpalWabbitThreadedPrediction(VowpalWabbitModel model)
            : base(model)
        {
        }

        /// <summary>
        /// Returns the same instance as no wrapping is required.
        /// </summary>
        /// <param name="vw">The wrapped vw instance.</param>
        protected override VowpalWabbit InternalCreate(VowpalWabbit vw)
        {
            return vw;
        }
    }

    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbit"/> instances.
    /// </summary>
    /// <typeparam name="TExample">The type use for providing data to VW using the serializer infrastructure.</typeparam>
    public sealed class VowpalWabbitThreadedPrediction<TExample> : VowpalWabbitThreadedPredictionBase<VowpalWabbit<TExample>>
    {
        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction"/>.
        /// </summary>
        /// <remarks>Decision Service client library needs default constructor.</remarks>
        public VowpalWabbitThreadedPrediction()
        {
        }

        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction{TExample}"/>.
        /// </summary>
        /// <param name="model">The model used by each pool instance.</param>
        public VowpalWabbitThreadedPrediction(VowpalWabbitModel model)
            : base(model)
        {
        }

        /// <summary>
        /// Creates a new instance of <see cref="VowpalWabbit{TExample}"/>.
        /// </summary>
        /// <param name="vw">The wrapped vw instance.</param>
        protected override VowpalWabbit<TExample> InternalCreate(VowpalWabbit vw = null)
        {
            return new VowpalWabbit<TExample>(vw);
        }
    }

    /// <summary>
    /// Enables multi-threaded prediction by utilizing a pool of <see cref="VowpalWabbit"/> instances.
    /// </summary>
    /// <typeparam name="TExample">The type use for providing data to VW using the serializer infrastructure.</typeparam>
    /// <typeparam name="TActionDependentFeature">The type use for providing action dependent data to VW using the serializer infrastructure.</typeparam>
    public sealed class VowpalWabbitThreadedPrediction<TExample, TActionDependentFeature> : VowpalWabbitThreadedPredictionBase<VowpalWabbit<TExample, TActionDependentFeature>>
    {
        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction"/>.
        /// </summary>
        /// <remarks>Decision Service client library needs default constructor.</remarks>
        public VowpalWabbitThreadedPrediction()
        {
        }

        /// <summary>
        /// Initializes a new instance of <see cref="VowpalWabbitThreadedPrediction{TExample,TActionDependentFeature}"/>.
        /// </summary>
        /// <param name="model">The model used by each pool instance.</param>
        public VowpalWabbitThreadedPrediction(VowpalWabbitModel model)
            : base(model)
        {
        }

        /// <summary>
        /// Creates a new instance of <see cref="VowpalWabbit{TExample}"/>.
        /// </summary>
        /// <param name="vw">The wrapped vw instance.</param>
        protected override VowpalWabbit<TExample, TActionDependentFeature> InternalCreate(VowpalWabbit vw)
        {
            return new VowpalWabbit<TExample, TActionDependentFeature>(vw);
        }
    }
}
