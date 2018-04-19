using System;
using System.Linq;
using System.IO;
using System.Globalization;

namespace Microsoft.DecisionService.Exploration
{
    public static class Program
    {
        public static int Main(string[] args)
        {
            // use a python driver that compares input/output
            // driver: param input -> distribution output
            // loop over stdin, print stdout numbers or # for error
            if (args.Length != 2)
            {
                Console.Error.WriteLine("Usage: TestDriver.exe <inputfile> <expectedfile>");
                return -1;
            }

            string line;
            bool exceptionCaught = false;
            int lineNr = 0;
            using (var input = File.OpenText(args[0]))
            using (var expected = File.OpenText(args[1]))
            {
                lineNr++;
                while ((line = input.ReadLine()) != null)
                {
                    if (line.StartsWith("#"))
                        continue;

                    var lineExpected = expected.ReadLine();

                    //Console.WriteLine("Input:    " + line);
                    //Console.WriteLine("Expected: " + lineExpected);

                    var fields = line.Split(' ');
                    try
                    {
                        float[] probabilityDistribution;

                        switch (fields[0])
                        {
                            case "epsilongreedy":
                                probabilityDistribution = ExplorationStrategies.GenerateEpsilonGreedy(
                                    float.Parse(fields[1], CultureInfo.InvariantCulture),
                                    int.Parse(fields[2], CultureInfo.InvariantCulture),
                                    int.Parse(fields[3], CultureInfo.InvariantCulture));
                                break;
                            case "softmax":
                                probabilityDistribution = ExplorationStrategies.GenerateSoftmax(
                                   float.Parse(fields[1], CultureInfo.InvariantCulture),
                                   fields.Skip(2).Select(f => float.Parse(f, CultureInfo.InvariantCulture)).ToArray());
                                break;
                            case "bag":
                                probabilityDistribution = ExplorationStrategies.GenerateBag(
                                    fields.Skip(1).Select(f => int.Parse(f, CultureInfo.InvariantCulture)).ToArray());
                                break;
                            default:
                                throw new ArgumentException("Unknown exploration strategy: " + args[0]);
                        }

                        // validate results
                        var expectedProbs = lineExpected.Split(' ').Select(f => float.Parse(f, CultureInfo.InvariantCulture)).ToArray();
                        if (expectedProbs.Length != probabilityDistribution.Length)
                            throw new Exception("Expected and actual probability distribution length must match");

                        // Console.WriteLine("Actual: " + string.Join(" ", probabilityDistribution));

                        for (int i = 0; i < expectedProbs.Length; i++)
                            FuzzyEqual(expectedProbs[i], probabilityDistribution[i], 1e-4);
                    }
                    catch (Exception ex)
                    {
                        exceptionCaught = true;
                        Console.Error.WriteLine(string.Format("# lineNr: {0} {1} {2}", lineNr, ex.Message, ex.StackTrace));
                    }
                }
            }

            return exceptionCaught ? -1 : 0;
        }

        static void FuzzyEqual(double expected, double actual, double epsilon)
        {
            // from test/RunTests
            var delta = Math.Abs(expected - actual);

            if (delta > epsilon)
            {
                // We have a 'big enough' difference, but this difference
                // may still not be meaningful in all contexts:

                // Big numbers should be compared by ratio rather than
                // by difference

                // Must ensure we can divide (avoid div-by-0)
                if (Math.Abs(actual) <= 1.0)
                {
                    // If numbers are so small (close to zero),
                    // ($delta > $Epsilon) suffices for deciding that
                    // the numbers are meaningfully different
                    throw new Exception(string.Format("{0} vs {1}: delta={2} > Epsilon={3}",
                        expected, actual, delta, epsilon));
                }

                // Now we can safely divide (since abs($word2) > 0)
                // and determine the ratio difference from 1.0
                var ratio_delta = Math.Abs(expected / actual - 1.0);
                if (ratio_delta > epsilon)
                {
                    throw new Exception(string.Format("{0} vs {1}: delta={2} > Epsilon={3}",
                        expected, actual, delta, epsilon));
                }
            }
        }
    }
}