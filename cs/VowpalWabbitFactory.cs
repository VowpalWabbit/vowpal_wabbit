// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VW.Interfaces;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// <see cref="IObjectFactory{VowpalWabbit}"/> implementation to produce <see cref="VowpalWabbit"/> instances.
    /// </summary>
    public class VowpalWabbitFactory : VowpalWabbitFactoryBase<VowpalWabbit>
    {
        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitFactory"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        public VowpalWabbitFactory(VowpalWabbitModel model) : base(model)
        {
        }

        /// <summary>
        /// Creates a new <see cref="VowpalWabbit"/> instance using the shared model.
        /// </summary>
        /// <returns>A new <see cref="VowpalWabbit"/> instance.</returns>
        public override VowpalWabbit Create()
        {
            return new VowpalWabbit(this.model);
        }
    }

    /// <summary>
    /// <see cref="IObjectFactory{TVowpalWabbit}"/> implementation to produce <see cref="VowpalWabbit{TExample}"/> instances.
    /// </summary>
    public class VowpalWabbitFactory<TExample> : VowpalWabbitFactoryBase<VowpalWabbit<TExample>>
    {
        /// <summary>
        /// Serializer settings.
        /// </summary>
        private VowpalWabbitSerializerSettings settings;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitFactory{TExample}"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbitFactory(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model)
        {
            this.settings = settings;
        }

        /// <summary>
        /// Creates a new <see cref="VowpalWabbit{TExample}"/> instance using the shared model.
        /// </summary>
        /// <returns>A new <see cref="VowpalWabbit{TExample}"/> instance.</returns>
        public override VowpalWabbit<TExample> Create()
        {
            return new VowpalWabbit<TExample>(this.model, this.settings);
        }
    }

    /// <summary>
    /// <see cref="IObjectFactory{TVowpalWabbit}"/> implementation to produce <see cref="VowpalWabbitPredictor{TExample,TActionDependentFeature}"/> instances.
    /// </summary>
    public class VowpalWabbitPredictorFactory<TExample, TActionDependentFeature> : VowpalWabbitFactoryBase<VowpalWabbitPredictor<TExample, TActionDependentFeature>>
        where TExample : SharedExample, IActionDependentFeatureExample<TActionDependentFeature>
    {
        /// <summary>
        /// Serializer settings.
        /// </summary>
        private VowpalWabbitSerializerSettings settings;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitFactory{TExample, TActionDependentFeature}"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbitPredictorFactory(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model)
        {
            this.settings = settings;
        }

        /// <summary>
        /// Creates a new <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instance using the shared model.
        /// </summary>
        /// <returns>A new <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instance.</returns>
        public override VowpalWabbitPredictor<TExample, TActionDependentFeature> Create()
        {
            return new VowpalWabbitPredictor<TExample, TActionDependentFeature>(this.model, this.settings);
        }
    }

    /// <summary>
    /// <see cref="IObjectFactory{TVowpalWabbit}"/> implementation to produce <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instances.
    /// </summary>
    public class VowpalWabbitFactory<TExample, TActionDependentFeature> : VowpalWabbitFactoryBase<VowpalWabbit<TExample, TActionDependentFeature>>
        where TExample : SharedExample, IActionDependentFeatureExample<TActionDependentFeature>
        where TActionDependentFeature : IExample
    {
        /// <summary>
        /// Serializer settings.
        /// </summary>
        private VowpalWabbitSerializerSettings settings;

        /// <summary>
        /// Initializes a new <see cref="VowpalWabbitFactory{TExample, TActionDependentFeature}"/> instance.
        /// </summary>
        /// <param name="model">The shared model.</param>
        /// <param name="settings">The serializer settings.</param>
        public VowpalWabbitFactory(VowpalWabbitModel model, VowpalWabbitSerializerSettings settings = null)
            : base(model)
        {
            this.settings = settings;
        }

        /// <summary>
        /// Creates a new <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instance using the shared model.
        /// </summary>
        /// <returns>A new <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instance.</returns>
        public override VowpalWabbit<TExample, TActionDependentFeature> Create()
        {
            return new VowpalWabbit<TExample, TActionDependentFeature>(this.model, this.settings);
        }
    }
}
