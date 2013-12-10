using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

using Microsoft.Research.MachineLearning;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace cs_test
{

    class Program
    {
        static void Main(string[] args)
        {
            //RunFeaturesTest();
            //RunParserTest();
            //RunSpeedTest();
            RunFlatExampleTestEx();
            //RunVWParce_and_VWLearn();
        }

        private static void RunFeaturesTest()
        {
            // this usually requires that the library script to update train.w or its moral equivalent needs to have been run 
            IntPtr vw = VowpalWabbitInterface.Initialize("-q st --noconstant --quiet");
            IntPtr example = VowpalWabbitInterface.ReadExample(vw, "1 |s p^the_man w^the w^man |t p^un_homme w^un w^homme");
            float score = VowpalWabbitInterface.Learn(vw, example);
            VowpalWabbitInterface.FinishExample(vw, example);

            VowpalWabbitInterface.FEATURE_SPACE[] featureSpace = new VowpalWabbitInterface.FEATURE_SPACE[2];//maximum number of index spaces

            VowpalWabbitInterface.FEATURE[] sfeatures = new VowpalWabbitInterface.FEATURE[3];// the maximum number of features
            VowpalWabbitInterface.FEATURE[] tfeatures = new VowpalWabbitInterface.FEATURE[3];// the maximum number of features

            GCHandle pinnedsFeatures = GCHandle.Alloc(sfeatures, GCHandleType.Pinned);
            GCHandle pinnedtFeatures = GCHandle.Alloc(tfeatures, GCHandleType.Pinned);

            featureSpace[0].features = pinnedsFeatures.AddrOfPinnedObject();
            featureSpace[1].features = pinnedtFeatures.AddrOfPinnedObject();

            GCHandle pinnedFeatureSpace = GCHandle.Alloc(featureSpace, GCHandleType.Pinned);

            IntPtr featureSpacePtr = pinnedFeatureSpace.AddrOfPinnedObject();

            uint snum = VowpalWabbitInterface.HashSpace(vw, "s");
            featureSpace[0].name = (byte)'s';
            sfeatures[0].weight_index = VowpalWabbitInterface.HashFeature(vw, "p^the_man", snum);
            sfeatures[0].x = 1;
            // add the character "delta" to test unicode
            // do it as a string to test the marshaling is doing pinning correctly.
            const string s = "w^thew^man\u0394";
            sfeatures[1].weight_index = VowpalWabbitInterface.HashFeature(vw, s, snum);
            sfeatures[1].x = 1;
            sfeatures[2].weight_index = VowpalWabbitInterface.HashFeature(vw, "w^man", snum);
            sfeatures[2].x = 1;
            featureSpace[0].len = 3;

            uint tnum = VowpalWabbitInterface.HashSpace(vw, "t");
            featureSpace[1].name = (byte)'t';
            tfeatures[0].weight_index = VowpalWabbitInterface.HashFeature(vw, "p^un_homme", tnum);
            tfeatures[0].x = 1;
            tfeatures[1].weight_index = VowpalWabbitInterface.HashFeature(vw, "w^un", tnum);
            tfeatures[1].x = 1;
            tfeatures[2].weight_index = VowpalWabbitInterface.HashFeature(vw, "w^homme", tnum);
            tfeatures[2].x = 1;
            featureSpace[1].len = 3;

            IntPtr importedExample = VowpalWabbitInterface.ImportExample(vw, featureSpacePtr, featureSpace.Length);

            VowpalWabbitInterface.AddLabel(importedExample, 1);

            score = VowpalWabbitInterface.Learn(vw, importedExample);

            Console.Error.WriteLine("p2 = {0}", score);

            VowpalWabbitInterface.Finish(vw);

            // clean up the memory we allocated
            pinnedsFeatures.Free();
            pinnedtFeatures.Free();
            pinnedFeatureSpace.Free();
        }

        private static void RunParserTest()
        {
            IntPtr vw = VowpalWabbitInterface.Initialize("-q st -d 0002.dat -f out");

            VowpalWabbitInterface.StartParser(vw, false);

            int count = 0;
            IntPtr example = IntPtr.Zero;
            while (IntPtr.Zero != (example = VowpalWabbitInterface.GetExample(vw)))
            {
                IntPtr labelPtr = VowpalWabbitInterface.GetLabel(vw, example);
                VowpalWabbitInterface.LABEL label = new VowpalWabbitInterface.LABEL();
                label = (VowpalWabbitInterface.LABEL)Marshal.PtrToStructure(labelPtr, typeof(VowpalWabbitInterface.LABEL));

                count++;
                int featureSpaceLen = 0;
                IntPtr featureSpacePtr = VowpalWabbitInterface.ExportExample(vw, example, ref featureSpaceLen);

                VowpalWabbitInterface.FEATURE_SPACE[] featureSpace = new VowpalWabbitInterface.FEATURE_SPACE[featureSpaceLen];
                int featureSpace_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE_SPACE));

                for (int i = 0; i < featureSpaceLen; i++)
                {
                    IntPtr curfeatureSpacePos = new IntPtr(featureSpacePtr.ToInt32() + i * featureSpace_size);
                    featureSpace[i] = (VowpalWabbitInterface.FEATURE_SPACE)Marshal.PtrToStructure(curfeatureSpacePos, typeof(VowpalWabbitInterface.FEATURE_SPACE));

                    VowpalWabbitInterface.FEATURE[] feature = new VowpalWabbitInterface.FEATURE[featureSpace[i].len];
                    int feature_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE));
                    for (int j = 0; j < featureSpace[i].len; j++)
                    {
                        IntPtr curfeaturePos = new IntPtr((featureSpace[i].features.ToInt32() + j * feature_size));
                        feature[j] = (VowpalWabbitInterface.FEATURE)Marshal.PtrToStructure(curfeaturePos, typeof(VowpalWabbitInterface.FEATURE));
                    }
                }
                VowpalWabbitInterface.ReleaseFeatureSpace(featureSpacePtr, featureSpaceLen);

                float score = VowpalWabbitInterface.Learn(vw, example);
                VowpalWabbitInterface.FinishExample(vw, example);
            }

            VowpalWabbitInterface.EndParser(vw);
            VowpalWabbitInterface.Finish(vw);
        }

        private static void RunSpeedTest()
        {
            Console.WriteLine(DateTime.Now.Millisecond + DateTime.Now.Second * 1000 + DateTime.Now.Minute * 60 * 1000);

            //IntPtr vw = VowpalWabbitInterface.Initialize("--ngram 2 --skips 4 -l 0.25 -b 22 -d rcv1.train.raw.txt -f out");
            //IntPtr vw = VowpalWabbitInterface.Initialize("-d rcv1.train.raw.txt -b 22 --ngram 2 --skips 4 -l 0.25 -c");
            //IntPtr vw = VowpalWabbitInterface.Initialize("-d rcv1.train.raw.txt -c");
            IntPtr vw = VowpalWabbitInterface.Initialize("-d vw.dat");

            VowpalWabbitInterface.StartParser(vw, false);

            int count = 0;
            IntPtr example = IntPtr.Zero;
            Stopwatch s = Stopwatch.StartNew();
            while (IntPtr.Zero != (example = VowpalWabbitInterface.GetExample(vw)))
            {
                count++;

                float score = VowpalWabbitInterface.Learn(vw, example);
                VowpalWabbitInterface.FinishExample(vw, example);
            }
            s.Stop();

            long t1 = s.ElapsedMilliseconds;
            VowpalWabbitInterface.EndParser(vw);

            VowpalWabbitInterface.Finish(vw);

            Console.WriteLine(DateTime.Now.Millisecond + DateTime.Now.Second * 1000 + DateTime.Now.Minute * 60 * 1000);

            Debug.WriteLine("RunSpeedTest Elapsed Time: {0} ms", s.ElapsedMilliseconds);
            Console.WriteLine("RunSpeedTest Elapsed Time: {0} ms", s.ElapsedMilliseconds);
        }

        //private static void RunFlatExampleTest()
        //{
        //    IntPtr vw = VowpalWabbitInterface.Initialize("-q st -d 0002.dat -f out");

        //    VowpalWabbitInterface.StartParser(vw, false);

        //    int count = 0;
        //    IntPtr example = IntPtr.Zero;
        //    while (IntPtr.Zero != (example = VowpalWabbitInterface.GetExample(vw)))
        //    {
        //        count++;
        //        // make example flat
        //        IntPtr flatec = VowpalWabbitInterface.Flatten_Example(vw, example);
        //        VowpalWabbitInterface.FinishExample(vw, example);
        //    }

        //    VowpalWabbitInterface.EndParser(vw);
        //    VowpalWabbitInterface.Finish(vw);
        //}

        private static void RunFlatExampleTestEx()
        {
            //IntPtr vw = VowpalWabbitInterface.Initialize("-q st -d rcv1.train.raw.txt -f out");
            IntPtr vw = VowpalWabbitInterface.Initialize("-q st -d 0002.dat -f out");

            VowpalWabbitInterface.StartParser(vw, false);

            uint stride = VowpalWabbitInterface.Get_Stride(vw);
            uint mask = (1 << 18) - 1;

            int count = 0;
            IntPtr example = IntPtr.Zero;
            while (IntPtr.Zero != (example = VowpalWabbitInterface.GetExample(vw)))
            {
                count++;
                IntPtr flatec = IntPtr.Zero;
                // make example flat
                flatec = VowpalWabbitInterface.Flatten_Example(vw, example);
                if (IntPtr.Zero == flatec)
                {
                    continue;
                }
                VowpalWabbitInterface.FLAT_RAW_EXAMPLE flat = new VowpalWabbitInterface.FLAT_RAW_EXAMPLE();
                flat = (VowpalWabbitInterface.FLAT_RAW_EXAMPLE)Marshal.PtrToStructure(flatec, typeof(VowpalWabbitInterface.FLAT_RAW_EXAMPLE));

                // Get IntPtr data
                VowpalWabbitInterface.FLAT_EXAMPLE flatEx = new VowpalWabbitInterface.FLAT_EXAMPLE();

                flatEx.final_prediction = flat.final_prediction;
                flatEx.example_counter = flat.example_counter;
                flatEx.ft_offset = flat.ft_offset;
                flatEx.global_weight = flat.global_weight;
                flatEx.num_features = flat.num_features;

                flatEx.ld = new VowpalWabbitInterface.LABEL();
                flatEx.ld = (VowpalWabbitInterface.LABEL)Marshal.PtrToStructure(flat.ld, typeof(VowpalWabbitInterface.LABEL));
                if (flat.tag_len > 0)
                {
                    flatEx.tag = new byte[flat.tag_len];
                    Marshal.Copy(flat.tag, flatEx.tag, 0, (int)flat.tag_len);
                }

                IList<int> indices = new List<int>();
                if (flat.feature_map_len > 0)
                {
                    flatEx.feature_map = new VowpalWabbitInterface.FEATURE[flat.feature_map_len];
                    int feature_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE));
                    for (int i = 0; i < (int)flat.feature_map_len; i++)
                    {
                        IntPtr curfeaturePos = new IntPtr(flat.feature_map.ToInt32() + i * feature_size);
                        flatEx.feature_map[i] = (VowpalWabbitInterface.FEATURE)Marshal.PtrToStructure(curfeaturePos, typeof(VowpalWabbitInterface.FEATURE));

                        int val = ((int)(flatEx.feature_map[i].weight_index / stride)) & (int)mask;
                        indices.Add(val);

                        flatEx.feature_map[i].weight_index = (uint)val;
                    }
                }

                VowpalWabbitInterface.FreeFlattenExample(flatec);
                VowpalWabbitInterface.FinishExample(vw, example);
            }

            VowpalWabbitInterface.EndParser(vw);
            VowpalWabbitInterface.Finish(vw);
        }

        public class VWInstanceEx
        {
            public VowpalWabbitInterface.FEATURE_SPACE[] featureSpace;

            public VWInstanceEx(IntPtr vw, IntPtr ex)
            {
                if (IntPtr.Zero == vw ||
                    IntPtr.Zero == ex)
                    return;

                int featureSpaceLen = 0;
                IntPtr featureSpacePtr = VowpalWabbitInterface.ExportExample(vw, ex, ref featureSpaceLen);

                this.featureSpace = new VowpalWabbitInterface.FEATURE_SPACE[featureSpaceLen];
                int featureSpace_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE_SPACE));

                for (int i = 0; i < featureSpaceLen; i++)
                {
                    IntPtr curfeatureSpacePos = new IntPtr(featureSpacePtr.ToInt32() + i * featureSpace_size);
                    this.featureSpace[i] = (VowpalWabbitInterface.FEATURE_SPACE)Marshal.PtrToStructure(curfeatureSpacePos, typeof(VowpalWabbitInterface.FEATURE_SPACE));

                    VowpalWabbitInterface.FEATURE[] feature = new VowpalWabbitInterface.FEATURE[this.featureSpace[i].len];
                    int feature_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE));
                    for (int j = 0; j < this.featureSpace[i].len; j++)
                    {
                        IntPtr curfeaturePos = new IntPtr((this.featureSpace[i].features.ToInt32() + j * feature_size));
                        feature[j] = (VowpalWabbitInterface.FEATURE)Marshal.PtrToStructure(curfeaturePos, typeof(VowpalWabbitInterface.FEATURE));
                    }
                }

                VowpalWabbitInterface.ReleaseFeatureSpace(featureSpacePtr, featureSpaceLen);
            }
        }
        private static void RunVWParse_and_VWLearn()
        {
            // parse and cache
            IntPtr vw0 = VowpalWabbitInterface.Initialize(@"-d rcv1.train.raw.txt -c");
            VowpalWabbitInterface.StartParser(vw0, false);

            long instanceCount = 0;
            VWInstanceEx[] vwInstanceExs = new VWInstanceEx[781266];
            Stopwatch s = Stopwatch.StartNew();
            while (instanceCount < 781266)
            {
                IntPtr example = VowpalWabbitInterface.GetExample(vw0);

                if (IntPtr.Zero == example)
                    break;

                vwInstanceExs[instanceCount] = new VWInstanceEx(vw0, example);
                VowpalWabbitInterface.FinishExample(vw0, example);
                instanceCount++;
            }
            VowpalWabbitInterface.EndParser(vw0);
            VowpalWabbitInterface.Finish(vw0);

            // learn
            instanceCount = 0;
            IntPtr vw = VowpalWabbitInterface.Initialize(@"--quiet --random_seed 276518665 -f C:\\Users\\niruc\\AppData\\Local\\Temp\\outl1.tmp --readable_model C:\\Users\\niruc\\AppData\\Local\\Temp\\outl2.tmp");
            foreach (VWInstanceEx vwInstanceEx in vwInstanceExs)
            {
                VowpalWabbitInterface.FEATURE_SPACE[] featureSpace = new VowpalWabbitInterface.FEATURE_SPACE[vwInstanceEx.featureSpace.Length];
                GCHandle[] pinnedsFeatures = new GCHandle[vwInstanceEx.featureSpace.Length];
                for (int i = 0; i < vwInstanceEx.featureSpace.Length; i++)
                {
                    pinnedsFeatures[i] = GCHandle.Alloc(vwInstanceEx.featureSpace[i].features, GCHandleType.Pinned);
                    featureSpace[i].features = pinnedsFeatures[i].AddrOfPinnedObject();
                }
                GCHandle pinnedFeatureSpace = GCHandle.Alloc(featureSpace, GCHandleType.Pinned);
                IntPtr featureSpacePtr = pinnedFeatureSpace.AddrOfPinnedObject();

                IntPtr importedExample = VowpalWabbitInterface.ImportExample(vw, featureSpacePtr, vwInstanceEx.featureSpace.Length);
                VowpalWabbitInterface.Learn(vw, importedExample);
                VowpalWabbitInterface.FinishExample(vw, importedExample);

                for (int i = 0; i < vwInstanceEx.featureSpace.Length; i++)
                {
                    pinnedsFeatures[i].Free();
                }
                pinnedFeatureSpace.Free();
                importedExample = IntPtr.Zero;

                instanceCount++;
            }
            VowpalWabbitInterface.Finish(vw);

            Debug.WriteLine("Elapsed Time: {0} ms", s.ElapsedMilliseconds);
            Console.WriteLine("Elapsed Time: {0} ms", s.ElapsedMilliseconds);

        }
    }
}

