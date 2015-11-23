// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitMarshalContext.cs">
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
using VW;

namespace VW.Serializer
{
    /// <summary>
    /// Context containing state during example marshalling.
    /// </summary>
    public class VowpalWabbitMarshalContext : IDisposable
    {
        /// <summary>
        /// /// Initializes a new instance of the <see cref="VowpalWabbitMarshalContext"/> class.
        /// </summary>
        /// <param name="vw">The VW instance the example will be imported to.</param>
        public VowpalWabbitMarshalContext(VowpalWabbit vw)
        {
            this.VW = vw;

            this.StringExample = new StringBuilder();
            this.ExampleBuilder = new VowpalWabbitExampleBuilder(vw);
        }

        /// <summary>
        /// The VW instance the produce example will be imported to.
        /// </summary>
        public VowpalWabbit VW { get; private set; }

        /// <summary>
        /// See https://github.com/JohnLangford/vowpal_wabbit/wiki/Input-format for reference
        /// </summary>
        public StringBuilder StringExample { get; private set; }

        /// <summary>
        /// Used to build examples.
        /// </summary>
        public VowpalWabbitExampleBuilder ExampleBuilder { get; private set; }

        /// <summary>
        /// Used to build a namespace.
        /// </summary>
        public VowpalWabbitNamespaceBuilder NamespaceBuilder { get; set; }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.ExampleBuilder != null)
                {
                    this.ExampleBuilder.Dispose();
                    this.ExampleBuilder = null;
                }
            }
        }
    }
}
