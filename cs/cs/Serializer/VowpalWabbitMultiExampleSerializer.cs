using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW.Labels;

namespace VW.Serializer
{
    internal sealed class VowpalWabbitMultiExampleSerializer<TExample, TActionDependentFeature> : IVowpalWabbitSerializer<TExample>
    {
        private readonly VowpalWabbit vw;

        private readonly Func<TExample, IEnumerable<TActionDependentFeature>> adfAccessor;

        private VowpalWabbitSingleExampleSerializer<TExample> sharedSerializer;

        private VowpalWabbitSingleExampleSerializer<TActionDependentFeature> adfSerializer;

        internal VowpalWabbitMultiExampleSerializer(VowpalWabbit vw,
            VowpalWabbitSingleExampleSerializer<TExample> sharedSerializer,
            VowpalWabbitSingleExampleSerializer<TActionDependentFeature> adfSerializer,
            Func<TExample, IEnumerable<TActionDependentFeature>> adfAccessor)
        {
            // sharedSerializer can be null
            Contract.Requires(vw != null);
            Contract.Requires(adfSerializer != null);
            Contract.Requires(adfAccessor != null);

            this.vw = vw;
            this.sharedSerializer = sharedSerializer;
            this.adfSerializer = adfSerializer;
            this.adfAccessor = adfAccessor;
        }

        public bool EnableStringExampleGeneration
        {
            get { return this.vw.Settings.EnableStringExampleGeneration; }
        }

        public bool CachesExamples
        {
            get { return (this.sharedSerializer != null && this.sharedSerializer.CachesExamples) || this.adfSerializer.CachesExamples; }
        }

        public int GetNumberOfActionDependentExamples(TExample example)
        {
            var adfs = this.adfAccessor(example);
            return adfs == null ? 0 : adfs.Count();
        }

        /// <summary>
        /// Serializes the given <paramref name="example"/> to VW string format.
        /// </summary>
        /// <param name="example">The example to serialize.</param>
        /// <param name="label">The label to serialize.</param>
        /// <param name="index">The optional index of the example, the <paramref name="label"/> should be attributed to.</param>
        /// <param name="dictionary">Dictionary used for dictify operation.</param>
        /// <param name="fastDictionary">Dictionary used for dictify operation.</param>
        /// <returns>The resulting VW string.</returns>
        public string SerializeToString(TExample example, ILabel label = null, int? index = null, Dictionary<string, string> dictionary = null, Dictionary<object, string> fastDictionary = null)
        {
            var sb = new StringBuilder();

            if (this.sharedSerializer != null)
                sb.AppendLine(this.sharedSerializer.SerializeToString(example, SharedLabel.Instance, null, dictionary, fastDictionary));

            var adfCollection = this.adfAccessor(example);
            if (adfCollection != null)
            {
                var i = 0;
                foreach (var adfExample in adfCollection)
                {
                    sb.AppendLine(this.adfSerializer.SerializeToString(adfExample, index != null && i == index ? label : null, null, dictionary, fastDictionary));
                    i++;
                }
            }

            return sb.ToString();
        }

        public VowpalWabbitExampleCollection Serialize(TExample example, ILabel label = null, int? index = null)
        {
            VowpalWabbitExample shared = null;
            var adf = new List<VowpalWabbitExample>();

            try
	        {
                if (this.sharedSerializer != null)
                    shared = this.sharedSerializer.Serialize(example, SharedLabel.Instance);

                var adfCollection = this.adfAccessor(example);
                if (adfCollection != null)
                {
                    var i = 0;
                    foreach (var adfExample in adfCollection)
                    {
                        adf.Add(this.adfSerializer.Serialize(adfExample, index != null && i == index ? label : null));
                        i++;
                    }
                }

                return new VowpalWabbitMultiLineExampleCollection(this.vw, shared, adf.ToArray());
	        }
	        catch (Exception)
	        {
                if (shared != null)
                    shared.Dispose();

                foreach (var adfExample in adf)
                    adfExample.Dispose();

		        throw;
	        }
        }

        public void Dispose()
        {
            if (this.sharedSerializer != null)
            {
                this.sharedSerializer.Dispose();
                this.sharedSerializer = null;
            }

            if (this.adfSerializer != null)
            {
                this.adfSerializer.Dispose();
                this.adfSerializer = null;
            }
        }
    }
}
