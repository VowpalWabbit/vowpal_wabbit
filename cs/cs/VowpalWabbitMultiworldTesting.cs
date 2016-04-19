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
            this.vw = new VowpalWabbit<LearnedVsConstantPolicy>(new VowpalWabbitSettings("--multiworld_test f", modelStream: vwModel));
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

        public sealed class PoliciesPerformance
        {
            private float[] data;

            public PoliciesPerformance(float[] data)
            {
                this.data = data;
            }

            public float LearnedPolicy { get { return this.data[0]; } }

            public int NumConstantPolicies { get { return this.data.Length - 1; } }

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
