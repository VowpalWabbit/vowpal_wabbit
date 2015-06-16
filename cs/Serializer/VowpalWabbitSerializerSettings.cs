// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializerSettings.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer
{
    public class VowpalWabbitSerializerSettings
    {
        public VowpalWabbitSerializerSettings()
        {
            this.EnableExampleCaching = false;
            this.MaxExampleCacheSize = int.MaxValue;
        }

        /// <summary>
        /// Set to true to disable example caching. Defaults to true.
        /// </summary>
        public bool EnableExampleCaching { get; set; }

        /// <summary>
        /// Bounds the example cache. Defaults to int.MaxValue.
        /// </summary>
        public int MaxExampleCacheSize { get; set; }
    }
}
