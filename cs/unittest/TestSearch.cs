using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using VW;

namespace cs_unittest
{
    using Multiline = IEnumerable<string>;

    [TestClass]
    public class TestSearch : TestBase
    {
        public sealed class FileDriver : IDisposable
        {
            private string file;
            private StreamReader streamReader;

            public FileDriver(string file)
            {
                this.file = file;
                this.streamReader = RunTestsHelper.Open(file);
            }

            public void Reset()
            {
                this.streamReader.BaseStream.Seek(0, SeekOrigin.Begin);
            }

            public void ForEachMultiline(Action<Multiline> action)
            {
                Multiline currMultiline;
                while ((currMultiline = this.NextMultiline()) != null)
                {
                    action(currMultiline);
                }
            }

            public Multiline NextMultiline()
            {
                this.CheckDisposed();

                string currLine;
                List<string> multiline = null;

                while ((currLine = this.streamReader.ReadLine()) != null)
                {
                    multiline = multiline ?? new List<string>();

                    if (String.IsNullOrWhiteSpace(currLine))
                    {
                        break;
                    }
                    else
                    {
                        multiline.Add(currLine);
                    }
                }

                return multiline;
            }

            public void ForEachLine(Action<string> action)
            {
                string currLine;
                while ((currLine = this.NextLine()) != null)
                {
                    action(currLine);
                }
            }

            public string NextLine()
            {
                this.CheckDisposed();

                return this.streamReader.ReadLine();
            }

            private void CheckDisposed()
            {
                if (this.streamReader == null) throw new ObjectDisposedException($"MultilineFileDriver({this.file})");
            }

            public void Dispose()
            {
                Interlocked.Exchange(ref this.streamReader, null)?.Dispose();
            }
        }

        [TestMethod]
        public void Test_RunSequenceTask_WithoutBuiltinDriver()
        {
            const string input = "train-sets/sequencespan_data";

            const int bits = 24;
            const int passes = 4;
            string vwargs = $"-k -c -b {bits} --invariant --passes {passes} --search_rollout none --search_task sequencespan --search 7 --holdout_off";

            VowpalWabbit rawvw = new VowpalWabbit(vwargs);

            uint GetLabel(VowpalWabbitExample ex) => ex.GetPrediction(rawvw, VowpalWabbitPredictionType.Multiclass);

            void RunSearchPredict(Multiline multiline)
            {
                // Parse the examples explicitly, because there is no easy way to get multiple predictions spread
                // across multiple input examples in non-ADF/LDF mode

                // TPrediction rawvw.Predict(ml, TPredictionFactory); // << There does not exist an appropriate one for search
                                                                      // Mostly because the infrastructure is not there to pull out
                                                                      // predictions from multiple examples.

                List<VowpalWabbitExample> multiex = new List<VowpalWabbitExample>();
                try
                {
                    foreach (string line in multiline)
                    {
                        multiex.Add(rawvw.ParseLine(line));
                    }

                    rawvw.Predict(multiex);
                    uint[] labels = multiex.Select(GetLabel).ToArray();

                    CollectionAssert.AreEqual(new uint[] { 2, 1, 1, 2, 2, 1, 6, 7, 7, 7, 7, 1, 6, 4, 1 }, labels);
                }
                finally
                {
                    foreach (VowpalWabbitExample ex in multiex)
                    {
                        rawvw.ReturnExampleToPool(ex);
                    }
                }
            }

            using (FileDriver driver = new FileDriver(input))
            {
                int remainingPasses = passes;

                do
                {
                    driver.ForEachMultiline(rawvw.Learn);

                    rawvw.EndOfPass();
                    driver.Reset();
                } while (--remainingPasses > 0);

                driver.Reset();

                driver.ForEachMultiline(RunSearchPredict);
            }

            rawvw.Dispose();
        }
    }
}
