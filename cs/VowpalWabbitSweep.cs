// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSweep.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Labels;
using VW.Serializer;

namespace VW
{
    /// <summary>
    /// Sweeping wrapper for multiline examples. Designed to re-use allocated examples
    /// across multiple Vowpal Wabbit instances. So far plain parallelization yielded
    /// faster training times at least on a 20 core machine.
    /// </summary>
    /// <typeparam name="TExample">User example type.</typeparam>
    /// <typeparam name="TActionDependentFeature">Action dependent feature type.</typeparam>
    public class VowpalWabbitSweep<TExample, TActionDependentFeature> : IDisposable
    {
        private const int NumberOfVWInstancesSharingExamples = 1;

        private VowpalWabbit[] vws;

        private List<VowpalWabbitSettings> settings;

        private VowpalWabbitSerializer<TExample>[] serializers;

        private VowpalWabbitSerializer<TActionDependentFeature>[] actionDependentFeatureSerializers;

        public VowpalWabbitSweep(List<VowpalWabbitSettings> settings)
        {
            if (settings == null || settings.Count == 0)
                throw new ArgumentException("settings");

            Contract.EndContractBlock();

            // TODO: check that the sweeps are not across incompatible options.
            this.settings = settings;
            this.vws = settings.Select(setting => new VowpalWabbit(setting)).ToArray();

            var diffs = this.vws.Skip(1).Select(vw => vw.AreFeaturesCompatible(this.vws[0])).Where(e => e != null).ToList();
            if (diffs.Count > 0)
                throw new ArgumentException("Feature settings are not compatible for sweeping: " + string.Join(",", diffs));

            this.serializers = this.vws.Select(vw => VowpalWabbitSerializerFactory.CreateSerializer<TExample>(vw.Settings).Create(vw)).ToArray();

            this.actionDependentFeatureSerializers = this.vws.Select(vw => VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(vw.Settings).Create(vw)).ToArray();
        }

        public VowpalWabbit[] VowpalWabbits { get { return this.vws; } }

        public OrderablePartitioner<Tuple<int, int>> CreatePartitioner()
        {
            return Partitioner.Create(0, this.vws.Length, Math.Min(this.vws.Length, NumberOfVWInstancesSharingExamples));
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        public void Learn(int fromInclusive, int toExclusive, TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            if (fromInclusive != toExclusive - 1)
                throw new ArgumentException("fromInclusive");

            VowpalWabbitMultiLine.Execute(this.vws[fromInclusive], this.serializers[fromInclusive], this.actionDependentFeatureSerializers[fromInclusive], example, actionDependentFeatures,
                (examples, _, __) =>
                {
                    for (int i = fromInclusive; i < toExclusive; i++)
                    {
                        foreach (var ex in examples)
                        {
                            this.vws[i].Learn(ex);
                        }
                    }
                }, index, label);
        }

        public TActionDependentFeature[][] Predict(int fromInclusive, int toExclusive, TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            if (fromInclusive != toExclusive - 1)
                throw new ArgumentException("fromInclusive");

            var result = new TActionDependentFeature[toExclusive - fromInclusive][];

            VowpalWabbitMultiLine.Execute(this.vws[fromInclusive], this.serializers[fromInclusive], this.actionDependentFeatureSerializers[fromInclusive], example, actionDependentFeatures,
                (examples, validActionDependentFeatures, emptyActionDependentFeatures) =>
                {
                    for (int i = fromInclusive; i < toExclusive; i++)
                    {
                        // feed all examples for this block
                        foreach (var ex in examples)
                        {
                            this.vws[i].Predict(ex);
                        }

                        result[i - fromInclusive] = VowpalWabbitMultiLine.GetPrediction(this.vws[i], examples, validActionDependentFeatures, emptyActionDependentFeatures)
                            .Select(p => p.Item2).ToArray();
                    }
                }, index, label);

            return result;
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.vws != null)
                {
                    foreach (var vw in this.vws)
                    {
                        vw.Dispose();
                    }

                    this.vws = null;
                }

                if (this.serializers != null)
                {
                    foreach (var s in this.serializers)
                    {
                        s.Dispose();
                    }

                    this.serializers = null;
                }

                if (this.actionDependentFeatureSerializers != null)
                {
                    foreach (var s in this.actionDependentFeatureSerializers)
                    {
                        s.Dispose();
                    }

                    this.actionDependentFeatureSerializers = null;
                }
            }
        }
    }
}
