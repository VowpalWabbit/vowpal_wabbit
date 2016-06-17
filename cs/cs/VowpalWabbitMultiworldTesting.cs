// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitMultiworldTesting.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using VW.Labels;
using VW.Serializer.Attributes;

namespace VW
{
    public sealed class VowpalWabbitMultiworldTesting : IDisposable
    {
        private VowpalWabbit<LearnedVsConstantPolicy> vw;

        /// <summary>
        ///
        /// </summary>
        /// <param name="vwModel">Optional model to see multiworld testing</param>
        public VowpalWabbitMultiworldTesting(Stream vwModel = null)
        {
            var settings = vwModel == null ?
                new VowpalWabbitSettings("--multiworld_test f") :
                new VowpalWabbitSettings { ModelStream = vwModel };

            this.vw = new VowpalWabbit<LearnedVsConstantPolicy>(settings);
        }

        public PoliciesPerformance Evaluate(uint learnedAction, int numActions, ContextualBanditLabel label)
        {
            return new PoliciesPerformance(
                this.vw.Learn(
                    new LearnedVsConstantPolicy(learnedAction, numActions),
                    label,
                    VowpalWabbitPredictionType.Scalars));
        }

        public VowpalWabbit Native
        {
            get
            {
                return this.vw.Native;
            }
        }

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
                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }
            }
        }

        /// <summary>
        /// Contains evaluation result for constant policies and the currently learned policy.
        /// </summary>
        public sealed class PoliciesPerformance
        {
            private float[] data;

            /// <summary>
            /// Initializes a new <see cref="PoliciesPerformance"/> instance.
            /// </summary>
            /// <param name="data">The performance data. Learned policy is at position 0.</param>
            public PoliciesPerformance(float[] data)
            {
                this.data = data;
            }

            /// <summary>
            /// The performance of the learned policy.
            /// </summary>
            public float LearnedPolicy { get { return this.data[0]; } }

            /// <summary>
            /// The number of constant policies evaluated.
            /// </summary>
            public int NumConstantPolicies { get { return this.data.Length - 1; } }

            /// <summary>
            /// The performance of each constant policy.
            /// </summary>
            public IEnumerable<float> ConstantPolicies
            {
                get
                {
                    return this.data.Skip(1);
                }
            }
        }

        /// <summary>
        /// Must public for the serializer to work with.
        /// </summary>
        [EditorBrowsableAttribute(EditorBrowsableState.Never)]
        public sealed class LearnedVsConstantPolicy
        {
            private uint learnedAction;

            private int numConstantActions;

            internal LearnedVsConstantPolicy(uint learnedAction, int numConstantActions)
            {
                this.learnedAction = learnedAction;
                this.numConstantActions = numConstantActions;
            }

            [Feature(FeatureGroup = 'f')]
            public IEnumerable<uint> Actions
            {
                get
                {
                    yield return learnedAction;

                    for (uint i = 0; i < this.numConstantActions; i++)
                    {
                        yield return i + 1;
                    }
                }
            }
        }
    }
}
