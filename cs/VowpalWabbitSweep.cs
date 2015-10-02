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
    public class VowpalWabbitSweep<TExample, TActionDependentFeature> : IDisposable
    {
        private const int NumberOfVWInstancesSharingExamples = 5;

        private VowpalWabbit[] vws;

        private List<VowpalWabbitSettings> settings;

        private VowpalWabbitSerializer<TExample> serializer;

        private VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer;

        public VowpalWabbitSweep(List<VowpalWabbitSettings> settings)
        {
            if (settings == null || settings.Count == 0)
                throw new ArgumentException("settings");

            Contract.EndContractBlock();

            this.settings = settings;
            this.vws = settings.Select(setting => new VowpalWabbit(setting)).ToArray();

            // TODO: check that the sweeps are not across incompatible options.
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(settings[0]).Create(this.vws[0]);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(settings[0]).Create(this.vws[0]);
        }

        public VowpalWabbit[] VowpalWabbits { get { return this.vws; } }

        /// <summary>
        /// Learn from the given example and return the current prediction for it.
        /// </summary>
        /// <param name="example">The shared example.</param>
        /// <param name="actionDependentFeatures">The action dependent features.</param>
        /// <param name="index">The index of the example to learn within <paramref name="actionDependentFeatures"/>.</param>
        /// <param name="label">The label for the example to learn.</param>
        public void Learn(TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            var partitioner = Partitioner.Create(0, this.vws.Length, Math.Min(this.vws.Length, NumberOfVWInstancesSharingExamples));

            Parallel.ForEach(
                partitioner,
                new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
                range =>
                {
                    VowpalWabbitMultiLine.Execute(this.vws[range.Item1], this.serializer, this.actionDependentFeatureSerializer, example, actionDependentFeatures,
                        (examples, _, __) =>
                        {
                            for (int i = range.Item1; i < range.Item2; i++)
                            {
                                foreach (var ex in examples.Where(ex => !ex.IsNewLine))
                                {
                                    this.vws[i].Learn(ex);
                                }
                            }
                        }, index, label);
                });
        }

        public TActionDependentFeature[][] Predict(TExample example, IReadOnlyCollection<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
            var partitioner = Partitioner.Create(0, this.vws.Length, Math.Min(this.vws.Length, NumberOfVWInstancesSharingExamples));

            var result = new TActionDependentFeature[this.vws.Length][];

            var p = Parallel.ForEach(
                partitioner,
                new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
                range =>
                {
                    VowpalWabbitMultiLine.Execute(this.vws[range.Item1], this.serializer, this.actionDependentFeatureSerializer, example, actionDependentFeatures,
                        (examples, validActionDependentFeatures, emptyActionDependentFeatures) =>
                        {
                            for (int i = range.Item1; i < range.Item2; i++)
                            {
                                foreach (var ex in examples.Where(ex => !ex.IsNewLine))
                                {
                                    this.vws[i].Predict(ex);
                                    result[i] = VowpalWabbitMultiLine.GetPrediction(this.vws[i], examples, validActionDependentFeatures, emptyActionDependentFeatures);
                                }
                            }
                        }, index, label);
                });

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

                if (this.serializer != null)
                {
                    this.serializer.Dispose();
                    this.serializer = null;
                }

                if (this.actionDependentFeatureSerializer != null)
                {
                    this.actionDependentFeatureSerializer.Dispose();
                    this.actionDependentFeatureSerializer = null;
                }
            }
        }
    }
}
