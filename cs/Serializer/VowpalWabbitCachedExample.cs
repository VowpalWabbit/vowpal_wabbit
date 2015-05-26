using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer
{
    internal class VowpalWabbitCachedExample<TExample> : IVowpalWabbitExample
    {
        private readonly VowpalWabbitSerializer<TExample> serializer;
        private readonly IVowpalWabbitExample example;

        internal VowpalWabbitCachedExample(VowpalWabbitSerializer<TExample> serializer, TExample source, IVowpalWabbitExample example)
        {
            this.serializer = serializer;
            this.example = example;
            this.Source = source;
            this.LastRecentUse = DateTime.Now;
        }

        internal TExample Source { get; private; }

        internal DateTime LastRecentUse { get; set; }

        public float CostSensitivePrediction
        {
            get { return this.example.CostSensitivePrediction; }
        }

        public string Diff(IVowpalWabbitExample other, bool sameOrder)
        {
            return this.example.Diff(other, sameOrder);
        }

        public bool IsNewLine
        {
            get { return this.example.IsNewLine; }
        }

        public float Learn()
        {
            return this.example.Learn();
        }

        public int[] MultilabelPredictions
        {
            get { return this.example.MultilabelPredictions; }
        }

        public float[] TopicPredictions
        {
            get { return this.example.TopicPredictions; }
        }

        public float Predict()
        {
            return this.example.Predict();
        }

        public void Dispose()
        {
            this.serializer.ReturnExampleToCache(this);
        }
    }
}
