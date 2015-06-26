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

namespace VW.Serializer
{
    public class VowpalWabbitSerializerSettings
    {
        private int maxExampleCacheSize;

        public VowpalWabbitSerializerSettings()
        {
            this.EnableExampleCaching = true;
            this.maxExampleCacheSize = int.MaxValue;
        }

        /// <summary>
        /// Set to true to disable example caching. Defaults to true.
        /// </summary>
        public bool EnableExampleCaching { get; set; }

        /// <summary>
        /// Bounds the example cache. Defaults to int.MaxValue.
        /// </summary>
        public int MaxExampleCacheSize
        {
            get { return this.maxExampleCacheSize; }

            set
            {
                if (value < 1)
                {
                    throw new ArgumentOutOfRangeException("MaxExampleCacheSize must be >= 1");
                }

                this.maxExampleCacheSize = value;
            }
        }
    }
}
