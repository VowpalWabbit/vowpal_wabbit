using MultiWorldTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;

namespace BlackBoxTests
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                return;
            }

            var configList = (JContainer)JsonConvert.DeserializeObject(File.ReadAllText(args[0]));
            for (int i = 0; i < configList.Count; i++)
            {
                var config = (JObject)configList[i];

                switch (config["Type"].Value<int>())
                {
                    case 0:
                        TestPrg(config);
                        break;
                    case 1:
                        TestHash(config);
                        break;
                    case 2:
                        TestExplore(config);
                        break;
                }
            }
        }

        static void TestPrg(JObject config)
        {
            var seed = config["Seed"].Value<ulong>();
            var iterations = config["Iterations"].Value<int>();
            var uniformInterval = JsonConvert.DeserializeObject<Tuple<uint, uint>>(config["UniformInterval"].ToString());
            var outputFile = config["OutputFile"].Value<string>();

            var outputLines = new List<string>();
            var prg = new PRG(seed);

            for (int i = 0; i < iterations; i++)
            {
                if (uniformInterval != null)
                {
                    uint random = prg.UniformInt(uniformInterval.Item1, uniformInterval.Item2);
                    outputLines.Add(random.ToString());
                }
                else
                {
                    float random = prg.UniformUnitInterval();
                    outputLines.Add(random.ToString());
                }
            }

            File.AppendAllLines(outputFile, outputLines);
        }

        static void TestHash(JObject config)
        {

        }

        static void TestExplore(JObject config)
        { 
            
        }
    }

    class TestTuple
    {
        public uint Item1 { get; set; }
        public uint Item2 { get; set; }

    }
}
