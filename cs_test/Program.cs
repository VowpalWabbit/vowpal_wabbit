using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

using Microsoft.Research.MachineLearning;
using System.Runtime.InteropServices;

namespace cs_test
{
    class Program
    {
        static void Main(string[] args)
        {
            RunMultiPassTest();
            RunFeaturesTest();
        }

        private static void RunFeaturesTest()
        {
            // this usually requires that the library script to update train.w or its moral equivalent needs to have been run 
            IntPtr vw = VowpalWabbitInterface.Initialize("--hash all -q st --noconstant -i .\\..\\..\\..\\..\\library\\train.w");

            IntPtr example = VowpalWabbitInterface.ReadExample(vw, "|s p^the_man w^the w^man |t p^un_homme w^un w^homme");
            float score = VowpalWabbitInterface.Learn(vw, example);
            VowpalWabbitInterface.FinishExample(vw, example);

            uint nnum = VowpalWabbitInterface.HashSpace(vw, "p");
            uint fnum = VowpalWabbitInterface.HashFeature(vw, "s p^the_man w^the w^man", nnum);

            VowpalWabbitInterface.FEATURE_SPACE[] featureSpace = new VowpalWabbitInterface.FEATURE_SPACE[11];
            for (int i = 0; i < featureSpace.Length; i++)
            {
                featureSpace[i].name = (byte)('a' + i);

                // create an array of features for this feature space
                VowpalWabbitInterface.FEATURE[] features = new VowpalWabbitInterface.FEATURE[2];
                
                // allocate some space for the unmanaged copies of these features
                featureSpace[i].features = Marshal.AllocHGlobal(features.Length * Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE)));
                featureSpace[i].len = features.Length;
                
                // fill the features, and copy them to the allocated memory
                for (int j = 0; j < features.Length; j++)
                {
                    // fill the feature
                    features[j].x = 1.1F;
                    features[j].weight_index = 100;

                    // copy the feature to the unmanaged heap
                    Marshal.StructureToPtr(features[j], IntPtr.Add(featureSpace[i].features, j * Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE))), false);
                }
            }

            IntPtr importedExample = VowpalWabbitInterface.ImportExample(vw, featureSpace, featureSpace.Length);


            Console.Error.WriteLine("p2 = {0}", score);

            VowpalWabbitInterface.Finish(vw);

            // clean up the memory we allocated
            for (int i = 0; i < featureSpace.Length; i++)
                Marshal.FreeHGlobal(featureSpace[i].features);
        }




        static public void RunMultiPassTest()
        {
 
            const string testDir = ".\\..\\..\\..\\..\\test\\";
            const string outDir = ".\\..\\..\\..\\..\\test\\";  // or make separate output directory before running
            const string trainSet = testDir + "train-sets\\0001.dat";
            const string testSet = testDir + "test-sets\\0001.dat";
            const string predictFile = outDir + "0001.predict.tmp";
            const string cacheFile = outDir + "00001A.dat.cache ";
            const string modelFile = outDir + "00001.model";

            string initTrain = String.Format(
                " -k --cache_file {0} --passes 3 --hash all --invariant --ngram 3 --skips  1 -l 20 --initial_t 128000 --power_t 1 -f {1}",
                 cacheFile, modelFile);

            IntPtr vw = VowpalWabbitInterface.Initialize(initTrain);
            using (var sr = new StreamReader(trainSet))
            {
                string line;
                while ((line = sr.ReadLine()) != null)
                {
                    IntPtr example = VowpalWabbitInterface.ReadExample(vw, line);
                    float score = VowpalWabbitInterface.Learn(vw, example);
                    VowpalWabbitInterface.FinishExample(vw, example);
                }
            }
            VowpalWabbitInterface.Finish(vw);

            string initPredict = String.Format(" --invariant  --ngram 3 --skips  1 -t  -i {0}",modelFile);
            vw = VowpalWabbitInterface.Initialize(initPredict);
            using (var sw = new StreamWriter(predictFile))
            {
                var sr = new StreamReader(testSet);
                ;
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        IntPtr example = VowpalWabbitInterface.ReadExample(vw, line);
                        float score = VowpalWabbitInterface.Learn(vw, example);
                        VowpalWabbitInterface.FinishExample(vw, example);
                        sw.WriteLine("{0}", score);
                    }
                    sw.Flush();
                }
            }
            VowpalWabbitInterface.Finish(vw);
        }







    }
}
