// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVisitableFeatureContract.cs">
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
    [ContractClassFor(typeof(IVisitableFeature))]

    internal abstract class IVisitableFeatureContract : IVisitableFeature
    {
        public Action Visit
        {
            get 
            {
                Contract.Ensures(Contract.Result<Action>() != null);
                return null;
            }
        }

        public abstract string Namespace
        {
            get;
        }

        public abstract char? FeatureGroup
        {
            get;
        }

        public abstract string Name
        {
            get;
        }

        public bool Enumerize
        {
            get { throw new NotImplementedException(); }
        }

        public bool AddAnchor
        {
            get { throw new NotImplementedException(); }
        }
    }
}
