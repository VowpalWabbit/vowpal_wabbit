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
using VW.Labels;
using VW.Serializer;
using VW.Serializer.Visitors;

namespace VW
{
    public class VowpalWabbit<TExample> : IDisposable
    {
        private VowpalWabbit vw;

        private VowpalWabbitSerializer<TExample> serializer;

        private readonly VowpalWabbitSerializer<TExample> learnSerializer;

        public VowpalWabbit(String args) : this(new VowpalWabbit(args))
        { }

        public VowpalWabbit(VowpalWabbitSettings settings)
            : this(new VowpalWabbit(settings))
        { }

        public VowpalWabbit(VowpalWabbit vw)
        {
            this.vw = vw;
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(vw.Settings);

            this.learnSerializer = this.serializer.CachesExamples ? null : this.serializer;
        }

        public void Learn(TExample example, ILabel label)
        {
#if DEBUG
            // hot path
            if (this.learnSerializer == null)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            // in release this throws NullReferenceException instead of producing silently wrong results
            using (var ex = this.learnSerializer.Serialize(this.vw, example, label))
            {
                this.vw.Learn(ex);
            }
        }

        public TPrediction Learn<TPrediction>(TExample example, ILabel label, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory)
        {
#if DEBUG
            // hot path
            if (this.learnSerializer == null)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif
            using (var ex = this.learnSerializer.Serialize(this.vw, example, label))
            {
                return this.vw.Learn(ex, predictionFactory);
            }
        }

        public void Predict(TExample example, ILabel label = null)
        {
            using (var ex = this.serializer.Serialize(this.vw, example, label))
            {
                this.vw.Learn(ex);
            }
        }

        public TPrediction Predict<TPrediction>(TExample example, IVowpalWabbitPredictionFactory<TPrediction> predictionFactory, ILabel label = null)
        {
            using (var ex = this.serializer.Serialize(this.vw, example, label))
            {
                return this.vw.Learn(ex, predictionFactory);
            }
        }

        public VowpalWabbit Native { get { return this.vw; } }

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

                if (this.serializer != null)
                {
                    this.serializer.Dispose();
                    this.serializer = null;
                }
            }
        }
    }

    public class VowpalWabbit<TExample, TActionDependentFeature> : IDisposable
    {
        private VowpalWabbit vw;

        private VowpalWabbitSerializer<TExample> serializer;

        private VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureSerializer;

        private readonly VowpalWabbitSerializer<TActionDependentFeature> actionDependentFeatureLearnSerializer;

        public VowpalWabbit(VowpalWabbit vw)
        {
            this.vw = vw;
            this.serializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(vw.Settings);
            this.actionDependentFeatureSerializer = VowpalWabbitSerializerFactory.CreateSerializer<TActionDependentFeature>(vw.Settings);
            this.actionDependentFeatureLearnSerializer = this.actionDependentFeatureSerializer.CachesExamples ? null : this.actionDependentFeatureSerializer;
        }

        public VowpalWabbit(String args)
            : this(new VowpalWabbit(args))
        { }

        public VowpalWabbit(VowpalWabbitSettings settings)
            : this(new VowpalWabbit(settings))
        { }


        public VowpalWabbit Native { get { return this.vw; } }


        public void Learn(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
#if DEBUG
            // hot path
            if (this.actionDependentFeatureLearnSerializer == null)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            VowpalWabbitMultiLine.Learn(
                this.vw,
                this.serializer,
                this.actionDependentFeatureLearnSerializer,
                example,
                actionDependentFeatures,
                index,
                label);
        }

        public TActionDependentFeature[] LearnAndPredict(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
#if DEBUG
            // hot path
            if (this.actionDependentFeatureLearnSerializer == null)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            return VowpalWabbitMultiLine.LearnAndPredict(
                this.vw,
                this.serializer,
                this.actionDependentFeatureLearnSerializer,
                example,
                actionDependentFeatures,
                index,
                label);
        }

        public int[] LearnAndPredictIndex(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures, int index, ILabel label)
        {
#if DEBUG
            // hot path
            if (this.actionDependentFeatureLearnSerializer == null)
            {
                throw new NotSupportedException("Cached examples cannot be used for learning");
            }
#endif

            return VowpalWabbitMultiLine.LearnAndPredictIndex(
                this.vw,
                this.serializer,
                this.actionDependentFeatureLearnSerializer,
                example,
                actionDependentFeatures,
                index,
                label);
        }

        public int[] PredictIndex(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures)
        {
            return VowpalWabbitMultiLine.PredictIndex(
                this.vw,
                this.serializer,
                this.actionDependentFeatureSerializer,
                example,
                actionDependentFeatures);
        }

        public TActionDependentFeature[] Predict(TExample example, IEnumerable<TActionDependentFeature> actionDependentFeatures)
        {
            return VowpalWabbitMultiLine.Predict(
                this.vw,
                this.serializer,
                this.actionDependentFeatureSerializer,
                example,
                actionDependentFeatures);
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
