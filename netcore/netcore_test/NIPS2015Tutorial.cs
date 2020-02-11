using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;
using VW.Labels;
using VW.Serializer.Attributes;

namespace cs_test
{
    /// <summary>
    /// Code examples as demonstrated during NIPS 2015 Tutorial (slides: https://github.com/JohnLangford/vowpal_wabbit/wiki/Tutorial)
    /// </summary>
    public class NIPS2015Tutorial
    {
        public void BasicExample()
        {
            using (var vw = new VowpalWabbit("--quiet"))
            {
                vw.Learn("1 |f 13:3.9656971e-02 24:3.4781646e-02 69:4.6296168e-02");

                var prediction = vw.Predict("|f 13:3.9656971e-02 24:3.4781646e-02 69:4.6296168e-02", VowpalWabbitPredictionType.Scalar);
                vw.SaveModel("output.model");
            }
        }


        public class MyExample
        {
            [Feature(FeatureGroup = 'p')]
            public float Income { get; set; }

            [Feature(Enumerize = true)]
            public int Age { get; set; }
        }

        public static void AnnotationExample()
        {
            using (var vw = new VowpalWabbit<MyExample>(new VowpalWabbitSettings { EnableStringExampleGeneration = true }))
            {
                var ex = new MyExample { Income = 40, Age = 25 };
                var label = new SimpleLabel { Label = 1 };

                var str = vw.Serializer.Create(vw.Native).SerializeToString(ex, label);
                // 1 |p Income:4 |  Age25

                vw.Learn(ex, label);

                var prediction = vw.Predict(ex, VowpalWabbitPredictionType.Scalar);
            }
        }

        public static void MultiThreadedPrediction()
        {
            var example = new MyExample { Income = 40, Age = 25 };

            var vwModel = new VowpalWabbitModel("-t -i m1.model");
            using (var pool = new VowpalWabbitThreadedPrediction<MyExample>(vwModel))
            {
                // thread-safe
                using (var vw = pool.GetOrCreate())
                {
                    // vw.Value is not thread-safe
                    vw.Value.Predict(example);
                }

                // thread-safe
                pool.UpdateModel(new VowpalWabbitModel("-t -i m2.model"));
            }
        }

        public static async Task MultiThreadedLearning()
        {
            var example = new MyExample { Income = 40, Age = 25 };
            var label = new SimpleLabel { Label = 1 };

            var settings = new VowpalWabbitSettings
            {
                ParallelOptions = new ParallelOptions
                {
                    MaxDegreeOfParallelism = 16
                },
                ExampleCountPerRun = 2000,
                ExampleDistribution = VowpalWabbitExampleDistribution.RoundRobin
            };

            using (var vw = new VowpalWabbitThreadedLearning(settings))
            {
                using (var vwManaged = vw.Create<MyExample>())
                {
                    var prediction = await vwManaged.Learn(example, label, VowpalWabbitPredictionType.Scalar);
                }

                var saveModelTask = vw.SaveModel("m1.model");

                await vw.Complete();
                await saveModelTask;
            }
        }
    }
}
