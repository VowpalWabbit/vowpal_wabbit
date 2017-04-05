using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer;

namespace cs_unittest
{
    internal static class VowpalWabbitExampleValidator
    {
        internal static void Validate(string line, VowpalWabbitExample ex, IVowpalWabbitLabelComparator comparator, string args = null)
        {
            using (var vw = new VowpalWabbit(args))
            using (var strExample = vw.ParseLine(line))
            {
                var diff = strExample.Diff(vw, ex, comparator);
                Assert.IsNull(diff, diff + " generated string: '" + ex.VowpalWabbitString + "'");
            }
        }
    }

    internal sealed class VowpalWabbitExampleValidator<TExample> : IDisposable
    {
        private VowpalWabbit<TExample> vw;
        private VowpalWabbit<TExample> vwNative;
        private VowpalWabbit vwJson;
        private VowpalWabbitSingleExampleSerializerCompiler<TExample> compiler;
        private Action<VowpalWabbitMarshalContext, TExample, ILabel> serializer;
        private Action<VowpalWabbitMarshalContext, TExample, ILabel> serializerNative;
        private IVowpalWabbitSerializer<TExample> factorySerializer;

        private static string FixArgs(string args)
        {
            // remove model writing
            args = Regex.Replace(args, @"-f\s+[^ -]+", " ");

            // remove cache file
            args = Regex.Replace(args, @"-c\s+([^ -]+)?", " ");

            return args;
        }

        internal VowpalWabbitExampleValidator(string args) : this(new VowpalWabbitSettings(FixArgs(args)))
        {
        }

        internal VowpalWabbitExampleValidator(VowpalWabbitSettings settings)
        {
            var stringSettings = (VowpalWabbitSettings)settings.Clone();
            stringSettings.EnableStringExampleGeneration = true;

            this.vw = new VowpalWabbit<TExample>(stringSettings);

            var jsonSettings = (VowpalWabbitSettings)settings.Clone();
            jsonSettings.Arguments += " --json";
            this.vwJson = new VowpalWabbit(jsonSettings);

            this.compiler = this.vw.Serializer as VowpalWabbitSingleExampleSerializerCompiler<TExample>;
            if (this.compiler != null)
                this.serializer = this.compiler.Func(this.vw.Native);

            this.vwNative = new VowpalWabbit<TExample>(settings);

            this.compiler = this.vwNative.Serializer as VowpalWabbitSingleExampleSerializerCompiler<TExample>;
            if (compiler != null)
                this.serializerNative = this.compiler.Func(this.vwNative.Native);

            this.factorySerializer = VowpalWabbitSerializerFactory.CreateSerializer<TExample>(stringSettings).Create(this.vw.Native);
        }

        public void Validate(string line, TExample example, ILabel label = null)
        {
            IVowpalWabbitLabelComparator comparator;

            if (label == null || label == SharedLabel.Instance)
            {
                comparator = null;
            }
            else if (label is SimpleLabel)
            {
                comparator = VowpalWabbitLabelComparator.Simple;
            }
            else if (label is ContextualBanditLabel)
            {
                comparator = VowpalWabbitLabelComparator.ContextualBandit;
            }
            else
            {
                throw new ArgumentException("Label type not supported: " + label.GetType());
            }

            using (var context = new VowpalWabbitMarshalContext(this.vw.Native))
            using (var contextNative = new VowpalWabbitMarshalContext(this.vwNative.Native))
            {
                // validate string serializer
                this.serializer(context, example, label);
                this.serializerNative(contextNative, example, label);

                // natively parsed string example compared against:
                // (1) natively build example
                // (2) string serialized & natively parsed string example
                using (var strExample = this.vw.Native.ParseLine(line))
                using (var strConvertedExample = this.vw.Native.ParseLine(context.ToString()))
                using (var nativeExample = contextNative.ExampleBuilder.CreateExample())
                using (var nativeExampleWithString = this.factorySerializer.Serialize(example, label))
                {
                    var diff = strExample.Diff(this.vw.Native, strConvertedExample, comparator);
                    Assert.IsNull(diff, diff + " generated string: '" + context.ToString() + "'");

                    diff = strExample.Diff(this.vw.Native, nativeExample, comparator);
                    Assert.IsNull(diff, diff);

                    if (!strExample.IsNewLine)
                    {
                        Assert.IsFalse(string.IsNullOrEmpty(nativeExampleWithString.VowpalWabbitString));
                        Assert.IsFalse(string.IsNullOrEmpty(this.factorySerializer.SerializeToString(example, label)));
                    }

                    if (this.vw.Native.Settings.TypeInspector == JsonTypeInspector.Default)
                    {
                        var jsonStr = JsonConvert.SerializeObject(example);

                        using (var jsonSerializer = new VowpalWabbitJsonSerializer(this.vw.Native))
                        {
                            using (var jsonExample = jsonSerializer.ParseAndCreate(jsonStr, label))
                            {
                                var ex = ((VowpalWabbitSingleLineExampleCollection)jsonExample).Example;

                                diff = strExample.Diff(this.vw.Native, ex, comparator);
                                Assert.IsNull(diff, $"{diff}\n json: '{jsonStr}'");
                            }
                        }

                        List<VowpalWabbitExample> exampleList = null;

                        try
                        {
                            exampleList = this.vwJson.ParseJson(jsonStr);

                            Assert.AreEqual(1, exampleList.Count);

                            diff = strExample.Diff(this.vw.Native, exampleList[0], comparator);
                            Assert.IsNull(diff, $"{diff}\n json: '{jsonStr}'");
                        }
                        finally
                        {
                            if (exampleList != null)
                            {
                                foreach (var ex in exampleList)
                                    ex.Dispose();
                            }
                        }
                    }
                }
            }
        }

        public void Validate(IEnumerable<string> lines, TExample example, IVowpalWabbitLabelComparator labelComparator = null, ILabel label = null)
        {
            // natively parsed string example compared against:
            // (1) natively build example
            // (2) string serialized & natively parsed string example
            var strExamples = lines.Select(l => this.vw.Native.ParseLine(l)).ToArray();
            using (var nativeExampleWithString = (VowpalWabbitMultiLineExampleCollection)this.factorySerializer.Serialize(example, label))
            {
                var examplesToCompare = new List<VowpalWabbitExample>();

                if (nativeExampleWithString.SharedExample != null)
                    examplesToCompare.Add(nativeExampleWithString.SharedExample);

                examplesToCompare.AddRange(nativeExampleWithString.Examples);

                examplesToCompare = examplesToCompare.Where(e => !e.IsNewLine).ToList();

                Assert.AreEqual(strExamples.Length, examplesToCompare.Count);

                for (int i = 0; i < strExamples.Length; i++)
                {
                    var diff = strExamples[i].Diff(this.vw.Native, examplesToCompare[i], labelComparator);
                    Assert.IsNull(diff, diff + " generated string: '" + examplesToCompare[i].VowpalWabbitString + "'");
                }
            }

            foreach (var ex in strExamples)
                ex.Dispose();
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
                if (this.vw != null)
                {
                    this.vw.Dispose();
                    this.vw = null;
                }

                if (this.vwNative != null)
                {
                    this.vwNative.Dispose();
                    this.vwNative = null;
                }

                if (this.vwJson != null)
                {
                    this.vwJson.Dispose();
                    this.vwJson = null;
                }

                if (this.factorySerializer != null)
                {
                    this.factorySerializer.Dispose();
                    this.factorySerializer = null;
                }
            }
        }
    }
}
