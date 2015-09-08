// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespaceDenseContract.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Collections.Generic;
using System.Diagnostics.Contracts;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Contracts
{
    [ContractClassFor(typeof(INamespaceDense<>))]
    internal abstract class INamespaceDenseContract<T> : INamespaceDense<T>
    {
        public IFeature<IReadOnlyCollection<T>> DenseFeature
        {
	        get 
            {
                Contract.Ensures(Contract.Result<IFeature<IReadOnlyCollection<T>>>() != null);
                return null;
            }
        }

        public abstract string Name
        {
            get;
        }

        public abstract char? FeatureGroup
        {
            get;
        }
    }
}
