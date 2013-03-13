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
            RunFeaturesTest();
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

            VowpalWabbitInterface.Label_Data labelData = VowpalWabbitInterface.DefaultLabelData();
            labelData.label = 1;
            labelData.weight = 1;
            // Put it in the example
            VowpalWabbitInterface.StartOfExample startOfExample = (VowpalWabbitInterface.StartOfExample)Marshal.PtrToStructure(importedExample, typeof(VowpalWabbitInterface.StartOfExample));
            Marshal.StructureToPtr(labelData, startOfExample.labeldata, false);

            score = VowpalWabbitInterface.Learn(vw, importedExample);

            Console.Error.WriteLine("p2 = {0}", score);

            VowpalWabbitInterface.Finish(vw);

            // clean up the memory we allocated
            
            pinnedsFeatures.Free();
            pinnedtFeatures.Free();
            pinnedFeatureSpace.Free();
        }

    }
}
