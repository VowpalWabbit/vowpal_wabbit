// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitMarshalContext.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;

namespace VW.Serializer
{
    /// <summary>
    /// Context containing state during example marshalling.
    /// </summary>
    [DebuggerDisplay("{GetHashCode()}: {ToString()}")]
    public class VowpalWabbitMarshalContext : IDisposable
    {
        /// <summary>
        /// If true disposes the example builder. Otherwise it's not owned by this instance.
        /// </summary>
        private bool disposeExampleBuilder = false;

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitMarshalContext"/> class.
        /// </summary>
        /// <param name="vwPool">The VW instance the example will be imported to.</param>
        /// <param name="dictionary">Dictionary used for dictify operation.</param>
        /// <param name="fastDictionary">Dictionary used for dictify operation.</param>
        public VowpalWabbitMarshalContext(IVowpalWabbitExamplePool vwPool, Dictionary<string, string> dictionary = null, Dictionary<object, string> fastDictionary = null)
            : this(vwPool.Native, new VowpalWabbitExampleBuilder(vwPool), dictionary, fastDictionary)
        {
            disposeExampleBuilder = true;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="VowpalWabbitMarshalContext"/> class.
        /// </summary>
        /// <param name="vw">The VW instance the example will be imported to.</param>
        /// <param name="exampleBuilder">A shared example builder.</param>
        /// <param name="dictionary">Dictionary used for dictify operation.</param>
        /// <param name="fastDictionary">Dictionary used for dictify operation.</param>
        public VowpalWabbitMarshalContext(VowpalWabbit vw, VowpalWabbitExampleBuilder exampleBuilder, Dictionary<string, string> dictionary = null, Dictionary<object, string> fastDictionary = null)
        {
            this.VW = vw;
            this.ExampleBuilder = exampleBuilder;

            if (vw.Settings.EnableStringExampleGeneration)
            {
                this.StringExample = new StringBuilder();
                this.Dictionary = dictionary;
                this.FastDictionary = fastDictionary;
            }
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
        /// The VW string version of the label.
        /// </summary>
        public string StringLabel { get; set; }

        /// <summary>
        /// Creates the VW string for this example.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            if (this.StringExample == null)
                return null;

            var sb = new StringBuilder();
            if (this.StringLabel != null)
                sb.Append(this.StringLabel);

            if (this.StringExample.Length > 0)
            {
                if (sb.Length > 0)
                    sb.Append(' ');

                sb.Append(this.StringExample);
            }

            return sb.ToString();
        }

        /// <summary>
        /// Used if dictify is true. Maps from serialized feature to surrogate key.
        /// </summary>
        public Dictionary<string, string> Dictionary { get; private set; }

        /// <summary>
        /// Used if dictify is true. Maps from raw feature value (e.g. int[]) to serialized feature.
        /// </summary>
        public Dictionary<object, string> FastDictionary { get; private set; }

        /// <summary>
        /// Used to build examples.
        /// </summary>
        public VowpalWabbitExampleBuilder ExampleBuilder { get; private set; }

        /// <summary>
        /// Used to build a namespace.
        /// </summary>
        public VowpalWabbitNamespaceBuilder NamespaceBuilder { get; set; }

        /// <summary>
        /// Formats <paramref name="args"/> based on <paramref name="format"/> to the string example buffer.
        /// </summary>
        /// <param name="dictify">If true, performs dictionarization on the serialized string and inserts a surrogate.</param>
        /// <param name="format">The string format used to serialize <paramref name="args"/>.</param>
        /// <param name="args">The arguments to the string format operation.</param>
        public void AppendStringExample(bool dictify, string format, params object[] args)
        {
            if (this.StringExample != null)
            {
                var outputString = string.Format(CultureInfo.InvariantCulture, format, args);

                if (dictify && this.Dictionary != null)
                {
                    string surrogate;
                    if (!this.Dictionary.TryGetValue(outputString, out surrogate))
                    {
                        // prefix to avoid number parsing
                        surrogate = "d" + this.Dictionary.Count.ToString(CultureInfo.InvariantCulture);
                        this.Dictionary.Add(outputString, surrogate);
                    }

                    this.StringExample.AppendFormat(" {0}", surrogate);
                }
                else
                {
                    this.StringExample.Append(outputString);
                }
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
                if (this.ExampleBuilder != null && disposeExampleBuilder)
                {
                    this.ExampleBuilder.Dispose();
                    this.ExampleBuilder = null;
                }
            }
        }
    }
}
