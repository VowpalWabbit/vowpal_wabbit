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

        private VowpalWabbitSingleExampleSerializer<TExample>[] serializers;

        private VowpalWabbitSingleExampleSerializer<TActionDependentFeature>[] actionDependentFeatureSerializers;

        /// <summary>
        /// Initializes a new instance.
        /// </summary>
        /// <param name="settings">The list of settings to be used.</param>
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

            this.serializers = this.vws.Select(vw =>
                (VowpalWabbitSingleExampleSerializer<TExample>)VowpalWabbitSerializerFactory.CreateSerializer<TExample>(vw.Settings).Create(vw))
                .ToArray();

            this.actionDependentFeatureSerializers = this.vws.Select(vw =>
                (VowpalWabbitSingleExampleSerializer<TActionDependentFeature>)VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(vw.Settings).Create(vw))
                .ToArray();
        }

        /// <summary>
        /// The internally allocated VW instances.
        /// </summary>
        public VowpalWabbit[] VowpalWabbits { get { return this.vws; } }

        /// <summary>
        /// Creates a partioner used for parallel scenarios.
        /// </summary>
        /// <returns>An ordered partitioner.</returns>
        public OrderablePartitioner<Tuple<int, int>> CreatePartitioner()
        {
            return Partitioner.Create(0, this.vws.Length, Math.Min(this.vws.Length, NumberOfVWInstancesSharingExamples));
        }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="fromInclusive">Instance number to start from.</param>
        /// <param name="toExclusive">Instance number to end at.</param>
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

        /// <summary>
        /// Prediction sweep.
        /// </summary>
        /// <param name="fromInclusive"></param>
        /// <param name="toExclusive"></param>
        /// <param name="example"></param>
        /// <param name="actionDependentFeatures"></param>
        /// <param name="index"></param>
        /// <param name="label"></param>
        /// <returns></returns>
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
                            .Select(p => p.Feature).ToArray();
                    }
                }, index, label);

            return result;
        }

        /// <summary>
        /// Save all models with the given prfix.
        /// </summary>
        /// <param name="modelPrefix"></param>
        /// <returns></returns>
	    public List<string> SaveModels(string modelPrefix)
        {
            return this.vws.Select((vw, i) =>
            {
                var modelName = modelPrefix + "-" + i;
                vw.SaveModel(modelName);
                return modelName;
            })
            .ToList();
        } 


        /// <summary>
        /// Reload all models.
        /// </summary>
	    public void Reload()
        {
            foreach (var vw in this.vws)
            {
                vw.Reload();
            }
        } 


        /// <summary>
        /// Executes the given action on each VW instance.
        /// </summary>
        /// <param name="action">The action to execute.</param>
	    public void Execute(Action<VowpalWabbit, VowpalWabbitSingleExampleSerializer<TExample>, VowpalWabbitSingleExampleSerializer<TActionDependentFeature>, int> action)
        {
            Parallel.For(
                0, this.vws.Length,
                new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount / 2 },
                i => action(this.vws[i], this.serializers[i], this.actionDependentFeatureSerializers[i], i));
        } 


        /// <summary>
        /// Dispose resources.
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
