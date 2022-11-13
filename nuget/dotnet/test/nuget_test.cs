using System;
using System.Diagnostics;
using VW;

Console.WriteLine("Running VW C# test...");
var test = new BuilderTestClass();
test.TestBuilderSimple();
Console.WriteLine("Done");

public class BuilderTestClass
{
    public void TestBuilderSimple()
    {
        using (VowpalWabbit vw = new VowpalWabbit(""))
        {
            VowpalWabbitExample e;

            using (var exampleBuilder = new VowpalWabbitExampleBuilder(vw))
            using (var nsBuilder = exampleBuilder.AddNamespace('U'))
            {
                ulong nsHash = vw.HashSpace("User");
                nsBuilder.AddFeature(vw.HashFeature("e1", nsHash), 0.3425f);

                e = exampleBuilder.CreateExample();
            }

            Debug.Assert(e != null);
            foreach (var n in e)
            {
                Console.WriteLine($"+ ({n.Index})=>'{(char)n.Index}'");
                foreach (var f in n)
                {
                    Console.WriteLine($"-- {f.WeightIndex}:{f.X}");
                }
            }
        }
    }
}

