using cs_unittest.cbadf;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer;

namespace cs_unittest
{
    [TestClass]
    public class TestAllReduceClass : TestBase
    {
        private static void Ingest(VowpalWabbit vw, IEnumerable<List<string>> blocks)
        {
            foreach (var block in blocks)
            {
                vw.Learn(block);
            }

            vw.EndOfPass();
        }

        private static void Ingest(VowpalWabbitThreadedLearning vw, IEnumerable<List<string>> blocks)
        {
            foreach (var block in blocks)
            {
                vw.Learn(block);
            }
        }

        private static void Ingest(VowpalWabbitAsync<CbAdfShared, CbAdfAction> vw, IEnumerable<Tuple<CbAdfShared, List<CbAdfAction>, ContextualBanditLabel>> data)
        {
            foreach (var d in data)
            {
                vw.Learn(d.Item1, d.Item2, (int)d.Item3.Action, d.Item3);
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public async Task TestAllReduce()
        {
            var data = Enumerable.Range(1, 1000).Select(_ => Generator.GenerateShared(10)).ToList();

            var stringSerializerCompiler = (VowpalWabbitSingleExampleSerializerCompiler<CbAdfShared>)
                VowpalWabbitSerializerFactory.CreateSerializer<CbAdfShared>(new VowpalWabbitSettings { EnableStringExampleGeneration = true });
            var stringSerializerAdfCompiler = (VowpalWabbitSingleExampleSerializerCompiler<CbAdfAction>)
                VowpalWabbitSerializerFactory.CreateSerializer<CbAdfAction>(new VowpalWabbitSettings { EnableStringExampleGeneration = true });

            var stringData = new List<List<string>>();

            VowpalWabbitPerformanceStatistics statsExpected;
            using (var spanningTree = new SpanningTreeClr())
            {
                spanningTree.Start();

                using (var vw1 = new VowpalWabbit(new VowpalWabbitSettings(@"--total 2 --node 1 --unique_id 0 --span_server localhost --cb_adf --rank_all --interact xy") { EnableStringExampleGeneration = true }))
                using (var vw2 = new VowpalWabbit(new VowpalWabbitSettings(@"--total 2 --node 0 --unique_id 0 --span_server localhost --cb_adf --rank_all --interact xy") { EnableStringExampleGeneration = true } ))
                {
                    var stringSerializer = stringSerializerCompiler.Func(vw1);
                    var stringSerializerAdf = stringSerializerAdfCompiler.Func(vw1);

                    // serialize
                    foreach (var d in data)
                    {
                        var block = new List<string>();

                        using (var context = new VowpalWabbitMarshalContext(vw1))
                        {
                            stringSerializer(context, d.Item1, SharedLabel.Instance);
                            block.Add(context.ToString());
                        }

                        block.AddRange(d.Item2.Select((a, i) =>
                            {
                                using (var context = new VowpalWabbitMarshalContext(vw1))
                                {
                                    stringSerializerAdf(context, a, i == d.Item3.Action ? d.Item3 : null);
                                    return context.ToString();
                                }
                            }));

                        stringData.Add(block);
                    }

                    await Task.WhenAll(
                        Task.Factory.StartNew(() => Ingest(vw1, stringData.Take(500))),
                        Task.Factory.StartNew(() => Ingest(vw2, stringData.Skip(500))));

                    vw1.SaveModel("expected.1.model");
                    vw2.SaveModel("expected.2.model");

                    statsExpected = vw1.PerformanceStatistics;
                }
            }

            // skip header
            var expected1Model = File.ReadAllBytes("expected.1.model").Skip(0x15).ToList();
            var expected2Model = File.ReadAllBytes("expected.2.model").Skip(0x15).ToList();

            var settings = new VowpalWabbitSettings("--cb_adf --rank_all --interact xy")
            {
                ParallelOptions = new ParallelOptions
                {
                    MaxDegreeOfParallelism = 2
                },
                ExampleCountPerRun = 2000,
                ExampleDistribution = VowpalWabbitExampleDistribution.RoundRobin
            };

            using (var vw = new VowpalWabbitThreadedLearning(settings))
            {
                await Task.WhenAll(
                    Task.Factory.StartNew(() => Ingest(vw, stringData.Take(500))),
                    Task.Factory.StartNew(() => Ingest(vw, stringData.Skip(500))));

                // important to enqueue the request before Complete() is called
                var statsTask = vw.PerformanceStatistics;
                var modelSave = vw.SaveModel("actual.model");

                await vw.Complete();

                var statsActual = await statsTask;
                VWTestHelper.AssertEqual(statsExpected, statsActual);

                await modelSave;

                // skip header
                var actualModel = File.ReadAllBytes("actual.model").Skip(0x15).ToList();

                CollectionAssert.AreEqual(expected1Model, actualModel);
                CollectionAssert.AreEqual(expected2Model, actualModel);
            }

            using (var vw = new VowpalWabbitThreadedLearning(settings))
            {
                var vwManaged = vw.Create<CbAdfShared, CbAdfAction>();

                await Task.WhenAll(
                    Task.Factory.StartNew(() => Ingest(vwManaged, data.Take(500))),
                    Task.Factory.StartNew(() => Ingest(vwManaged, data.Skip(500))));

                // important to enqueue the request before Complete() is called
                var statsTask = vw.PerformanceStatistics;
                var modelSave = vw.SaveModel("actual.managed.model");

                await vw.Complete();

                var statsActual = await statsTask;
                VWTestHelper.AssertEqual(statsExpected, statsActual);

                await modelSave;

                // skip header
                var actualModel = File.ReadAllBytes("actual.managed.model").Skip(0x15).ToList();

                CollectionAssert.AreEqual(expected1Model, actualModel);
                CollectionAssert.AreEqual(expected2Model, actualModel);
            }
        }
    }
}
