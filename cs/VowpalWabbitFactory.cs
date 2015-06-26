// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VW.Interfaces;

namespace VW
{
    /// <summary>
    /// <see cref="Microsoft.Research.MachineLearning.Interfaces.IObjectFactory"/> implementation to produce <see cref="VowpalWabbit"/> instances.
    /// </summary>
    public class VowpalWabbitFactory : VowpalWabbitFactoryBase<VowpalWabbit>
    {
        public VowpalWabbitFactory(VowpalWabbitModel model) : base(model)
        {
        }

        public override VowpalWabbit Create()
        {
            return new VowpalWabbit(this.model);
        }
    }

    /// <summary>
    /// <see cref="Microsoft.Research.MachineLearning.Interfaces.IObjectFactory"/> implementation to produce <see cref="VowpalWabbit{TExample}"/> instances.
    /// </summary>
    public class VowpalWabbitFactory<TExample> : VowpalWabbitFactoryBase<VowpalWabbit<TExample>>
    {
        public VowpalWabbitFactory(VowpalWabbitModel model)
            : base(model)
        {
        }

        public override VowpalWabbit<TExample> Create()
        {
            return new VowpalWabbit<TExample>(this.model);
        }
    }

    /// <summary>
    /// <see cref="Microsoft.Research.MachineLearning.Interfaces.IObjectFactory"/> implementation to produce <see cref="VowpalWabbit{TExample,TActionDependentFeature}"/> instances.
    /// </summary>
    public class VowpalWabbitFactory<TExample, TActionDependentFeature> : VowpalWabbitFactoryBase<VowpalWabbit<TExample, TActionDependentFeature>>
        where TExample : SharedExample, IActionDependentFeatureExample<TActionDependentFeature>
    {
        public VowpalWabbitFactory(VowpalWabbitModel model)
            : base(model)
        {
        }

        public override VowpalWabbit<TExample, TActionDependentFeature> Create()
        {
            return new VowpalWabbit<TExample, TActionDependentFeature>(this.model);
        }
    }
}
