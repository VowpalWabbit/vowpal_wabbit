using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer;

namespace cs_unittest
{
    internal sealed class VowpalWabbitExampleJsonValidator : IDisposable
    {
        private VowpalWabbit vw;

        internal VowpalWabbitExampleJsonValidator(string args = null) : this(new VowpalWabbitSettings((args ?? "") + " --json"))
        {
        }

        internal VowpalWabbitExampleJsonValidator(VowpalWabbitSettings settings)
        {
            settings = (VowpalWabbitSettings)settings.Clone();
            settings.EnableStringExampleGeneration = true;
            this.vw = new VowpalWabbit(settings);
        }

        public void Validate(string line, VowpalWabbitExampleCollection example, IVowpalWabbitLabelComparator labelComparator = null, ILabel label = null, bool validateVowpalWabbitString = true)
        {
            Assert.IsNotNull(example);

            var jsonExample = example as VowpalWabbitSingleLineExampleCollection;
            Assert.IsNotNull(jsonExample);

            using (var strExample = this.vw.ParseLine(line))
            {
                var diff = strExample.Diff(this.vw, jsonExample.Example, labelComparator);
                Assert.IsNull(diff, diff + " generated string: '" + jsonExample.VowpalWabbitString + "'");

                if (validateVowpalWabbitString)
                {
                    using (var strJsonExample = this.vw.ParseLine(jsonExample.Example.VowpalWabbitString))
                    {
                        diff = strExample.Diff(this.vw, strJsonExample, labelComparator);
                        Assert.IsNull(diff, diff);
                    }
                }
            }
        }

        public void Validate(string line, string json, IVowpalWabbitLabelComparator labelComparator = null, ILabel label = null, bool enableNativeJsonValidation = true)
        {
            using (var jsonSerializer = new VowpalWabbitJsonSerializer(this.vw))
            using (var jsonExample = jsonSerializer.ParseAndCreate(json, label))
            {
                this.Validate(line, jsonExample, labelComparator, label);

                if (enableNativeJsonValidation)
                {
                    var examples = this.vw.ParseJson(json);
                    Assert.AreEqual(1, examples.Count);
                    using (var jsonNativeExample = new VowpalWabbitSingleLineExampleCollection(this.vw, examples[0]))
                    {
                        this.Validate(line, jsonNativeExample, labelComparator, label, validateVowpalWabbitString: false);
                    }
                }
            }
        }


        public void Validate(string[] lines, JsonReader jsonReader, IVowpalWabbitLabelComparator labelComparator = null, ILabel label = null, int? index = null, VowpalWabbitJsonExtension extension = null)
        {
            VowpalWabbitExample[] strExamples = new VowpalWabbitExample[lines.Count()];

            try
            {
                for (int i = 0; i < lines.Length; i++)
                    strExamples[i] = this.vw.ParseLine(lines[i]);

                using (var jsonSerializer = new VowpalWabbitJsonSerializer(this.vw))
                {
                    if (extension != null)
                    {
                        jsonSerializer.RegisterExtension(extension);
                        // extension are not supported with native JSON parsing
                    }

                    using (var jsonExample = (VowpalWabbitMultiLineExampleCollection)jsonSerializer.ParseAndCreate(jsonReader, label, index))
                    {
                        var jsonExamples = new List<VowpalWabbitExample>();

                        if (jsonExample.SharedExample != null)
                            jsonExamples.Add(jsonExample.SharedExample);

                        jsonExamples.AddRange(jsonExample.Examples);

                        Assert.AreEqual(strExamples.Length, jsonExamples.Count);


                        for (int i = 0; i < strExamples.Length; i++)
                        {
                            using (var strJsonExample = this.vw.ParseLine(jsonExamples[i].VowpalWabbitString))
                            {
                                var diff = strExamples[i].Diff(this.vw, jsonExamples[i], labelComparator);
                                Assert.IsNull(diff, diff + " generated string: '" + jsonExamples[i].VowpalWabbitString + "'");

                                diff = strExamples[i].Diff(this.vw, strJsonExample, labelComparator);
                                Assert.IsNull(diff, diff);
                            }
                        }
                    }
                }
            }
            finally
            {
                foreach (var ex in strExamples)
                    if (ex != null)
                        ex.Dispose();
            }
        }

        public void Validate(string[] lines, List<VowpalWabbitExample> examples, IVowpalWabbitLabelComparator labelComparator = null)
        {
            VowpalWabbitExample[] strExamples = new VowpalWabbitExample[lines.Count()];
            try
            {
                for (int i = 0; i < lines.Length; i++)
                    strExamples[i] = this.vw.ParseLine(lines[i]);

                for (int i = 0; i < strExamples.Length; i++)
                {
                        var diff = strExamples[i].Diff(this.vw, examples[i], labelComparator);
                        Assert.IsNull(diff, diff + " generated string: '" + strExamples[i].VowpalWabbitString + "'");
                }
            }
            finally
            {
                foreach (var ex in strExamples)
                    if (ex != null)
                        ex.Dispose();
            }
        }

        public void Validate(string[] lines, string json, IVowpalWabbitLabelComparator labelComparator = null, ILabel label = null, int? index = null, VowpalWabbitJsonExtension extension = null, bool enableNativeJsonValidation = true)
        {
            VowpalWabbitExample[] strExamples = new VowpalWabbitExample[lines.Count()];

            try
            {
                for (int i = 0; i < lines.Length; i++)
                    strExamples[i] = this.vw.ParseLine(lines[i]);

                using (var jsonSerializer = new VowpalWabbitJsonSerializer(this.vw))
                {
                    if (extension != null)
                    {
                        jsonSerializer.RegisterExtension(extension);
                        // extension are not supported with native JSON parsing
                        enableNativeJsonValidation = false;
                    }

                    List<VowpalWabbitExample> jsonNativeExamples = null;

                    try
                    {
                        if (enableNativeJsonValidation)
                        {
                            jsonNativeExamples = this.vw.ParseJson(json);
                            Assert.IsNotNull(jsonNativeExamples);
                        }

                        using (var jsonExample = (VowpalWabbitMultiLineExampleCollection)jsonSerializer.ParseAndCreate(json, label, index))
                        {
                            var jsonExamples = new List<VowpalWabbitExample>();

                            if (jsonExample.SharedExample != null)
                                jsonExamples.Add(jsonExample.SharedExample);

                            jsonExamples.AddRange(jsonExample.Examples);

                            Assert.AreEqual(strExamples.Length, jsonExamples.Count);
                            if (enableNativeJsonValidation)
                                Assert.AreEqual(strExamples.Length, jsonNativeExamples.Count);

                            for (int i = 0; i < strExamples.Length; i++)
                            {
                                using (var strJsonExample = this.vw.ParseLine(jsonExamples[i].VowpalWabbitString))
                                {
                                    var diff = strExamples[i].Diff(this.vw, jsonExamples[i], labelComparator);
                                    Assert.IsNull(diff, diff + " generated string: '" + jsonExamples[i].VowpalWabbitString + "'");

                                    diff = strExamples[i].Diff(this.vw, strJsonExample, labelComparator);
                                    Assert.IsNull(diff, diff);

                                    if (enableNativeJsonValidation)
                                    {
                                        diff = strExamples[i].Diff(this.vw, jsonNativeExamples[i], labelComparator);
                                        Assert.IsNull(diff, diff);
                                    }
                                }
                            }
                        }
                    }
                    finally
                    {
                        if (jsonNativeExamples != null)
                        {
                            foreach (var ex in jsonNativeExamples)
                                ex.Dispose();
                        }
                    }
                }
            }
            finally
            {
                foreach (var ex in strExamples)
                    if (ex != null)
                        ex.Dispose();
            }
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
            }
        }
    }
}
