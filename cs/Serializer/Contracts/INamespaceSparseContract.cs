// --------------------------------------------------------------------------------------------------------------------
// <copyright file="INamespaceSparseContract.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Diagnostics.Contracts;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Contracts
{
    [ContractClassFor(typeof(INamespaceSparse))]
    internal abstract class INamespaceSparseContract : INamespaceSparse
    {
        public IVisitableFeature[] Features
        {
            get 
            {
                Contract.Ensures(Contract.Result<IVisitableFeature[]>() != null);
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
