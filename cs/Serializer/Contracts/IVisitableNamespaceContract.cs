// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVisitableNamespaceContract.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Diagnostics.Contracts;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Contracts
{
    [ContractClassFor(typeof(IVisitableNamespace))]
    internal abstract class IVisitableNamespaceContract : IVisitableNamespace
    {
        public Action Visit
        {
            get
            {
                Contract.Ensures(Contract.Result<Action>() != null);
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
