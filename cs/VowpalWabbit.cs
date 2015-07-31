// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbit.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using VW;
using VW.Interfaces;
using VW.Serializer;
using VW.Serializer.Visitors;

namespace VW
{
    ///*
    //public interface IVowpalWabbitSource
    //{
    //    IVowpalWabbitNative Acquire();

    //    void Release(IVowpalWabbitNative vw);
    //    /*
    //    VowpalWabbit<TExample> Create<TExample>();

    //    VowpalWabbit<TExample, TActionDependentFeature> Create<TExample, TActionDependentFeature>();
    //     * */
    //}

    ///// <summary>
    ///// A wrapper for Vowpal Wabbit using a native serializer transferring data using the library interface.
    ///// </summary>
    ///// <typeparam name="TExample">The user example type.</typeparam>
    //public class VowpalWabbit : VowpalWabbitNative, IVowpalWabbitSource
    //{
    //    /// <summary>
    //    /// Initializes a new <see cref="VowpalWabbit{TExample}"/> instance.
    //    /// </summary>
    //    public VowpalWabbit(VowpalWabbitSettings settings) : base(settings)
    //    {
    //    }

    //    public VowpalWabbit<TExample> Create<TExample>()
    //    {
    //        return new VowpalWabbit<TExample>(this.vw);
    //    }

    //    public VowpalWabbit<TExample, TActionDependentFeature> Create<TExample, TActionDependentFeature>()
    //    {
    //        return new VowpalWabbit<TExample, TActionDependentFeature>(this, this.Settings);
    //    }

    //    public IVowpalWabbitNative Acquire()
    //    {
    //        return this;
    //    }

    //    public void Release(IVowpalWabbitNative vw)
    //    {
    //    }
    //}
    //*/
    //public class VowpalWabbit<TExample> : IDisposable
    //{
    //    /// <summary>
    //    /// The serializer for the example user type.
    //    /// </summary>
    //    private readonly VowpalWabbitSerializer<TExample> serializer;

    //    /// <param name="settings">The serializer settings.</param>
    //    internal VowpalWabbit(VowpalWabbitSettings settings)
    //    {
    //        this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(settings);
    //    }

    //    /// <summary>
    //    /// Serializes <paramref name="example"/> into VowpalWabbit.
    //    /// </summary>
    //    /// <param name="example">The example to be read.</param>
    //    /// <returns>A native Vowpal Wabbit representation of the example.</returns>
    //    public IVowpalWabbitExample ReadExample(VowpalWabbitInterfaceVisitor visitor, TExample example, ILabel label = null)
    //    {
    //        return this.serializer.Serialize(example, label);    
    //    }

    //    /// <summary>
    //    /// Cleanup.
    //    /// </summary>
    //    /// <param name="isDiposing">See IDiposable pattern.</param>
    //    protected override void Dispose(bool isDiposing)
    //    {
    //        if (isDiposing)
    //        {
    //            if (this.serializer != null)
    //            {
    //                // free cached examples
    //                this.serializer.Dispose();
    //                this.serializer = null;
    //            }
    //        }

    //        // don't dispose VW before we can dispose all cached examples
    //        base.Dispose(isDiposing);
    //    }
    //}

    public class VowpalWabbit<TExample, TActionDependentFeature> : IDisposable
    {
        /// <summary>
        /// The serializer for the example user type.
        /// </summary>
        private readonly VowpalWabbitSerializer<TExample> serializer;

        private readonly VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer;

        /// <param name="settings">The serializer settings.</param>
        internal VowpalWabbit(VowpalWabbitSettings settings)
        {
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(settings);

            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(settings);

            if (this.actionDependentFeatureSerializer == null)
            {
                throw new ArgumentException(typeof(TActionDependentFeature) + " must have a least a single [Feature] defined.");
            }

            using (var exBuilder = new VowpalWabbitExampleBuilder(vowpalWabbit))
            {
                this.emptyExample = exBuilder.CreateExample();
            }
        }



        /// <summary>
        /// Cleanup.
        /// </summary>
        /// <param name="isDiposing">See IDiposable pattern.</param>
        protected override void Dispose(bool isDiposing)
        {
            if (isDiposing)
            {
                if (this.actionDependentFeatureSerializer != null)
                {
                    this.actionDependentFeatureSerializer.Dispose();
                    this.actionDependentFeatureSerializer = null;
                }
                if (this.emptyExample != null)
                {
                    this.emptyExample.Dispose();
                    this.emptyExample = null;
                }
            }
            base.Dispose(isDiposing);
        }
    }
}
