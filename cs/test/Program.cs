using System;
using System.IO;
using VW;
using System.Runtime.InteropServices;
using System.Diagnostics;
using cs_unittest;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace cs_test
{

    class Program
    {
        static void Main(string[] args)
        {
            //NIPS2015Tutorial.AnnotationExample();
            //AttributesSample.Attributes();
            //AttributesSample.RunFeaturesTest();
            ////LabDemo.Run();
            //RunFeaturesTest();
            //RunParserTest();
            //RunSpeedTest();
            //RunFlatExampleTestEx();
            //RunLDAPredict();
            //RunVWParse_and_VWLearn();
            if(args.Length == 0 || args[0] == "-tests")
                RunVWTest();
            //RunUnitTests();
            if (args.Length > 0)
            {
                if (args[0] == "-parallel")
                    RunParallelVw();
            }
        }

        static void RunParallelVw()
        {
            int instances = 20;
            int samples = 200;
            //Setup some vw-instances
            List<VowpalWabbit> vwInstances = new List<VowpalWabbit>();
            for (int i = 0; i < instances; i++)
            {
                vwInstances.Add(new VowpalWabbit($"--oaa 3 --cache_file {i}.cache -k --early_terminate 10 --passes 10"));
            }

            string ex1 = "1 | some features here",
                   ex2 = "2 | some other features",
                   ex3 = "3 | some more";

            //Prepare some data
            for (int i = 0; i < samples; i++)
            {
                foreach (var vw in vwInstances)
                {
                    vw.Learn(ex1);
                    vw.Learn(ex2);
                    vw.Learn(ex3);
                }
            }
            Console.WriteLine("trained models, now closing");

            //The Part causing the Exception
            var res = Parallel.ForEach(vwInstances, vw => vw.RunMultiPass());
            Console.WriteLine("done {0}", res.IsCompleted);
        }
        private static void RunUnitTests()
        {
            TestCbAdfClass tw = new TestCbAdfClass();
            tw.TestCbAdfExplore();
        }

        private static void RunVWTest()
        {
            // TestJsonDictClass tst = new TestJsonDictClass();
            // tst.TestJsonDictThreading();

            //TestCbAdfClass tst = new TestCbAdfClass();
            //tst.TestCbAdfExplore();

            //TestCbAdfClass tst = new TestCbAdfClass();
            //tst.Test87();

            //RunTestsHelper.ExecuteTest(
            //    125,
            //    "-k -c -d train-sets/wsj_small.dparser.vw.gz -b 20 --search_task dep_parser --search 26 --search_alpha 1e-5 --search_rollin mix_per_roll --search_rollout oracle --one_learner --search_history_length 3 --root_label 8 --transition_system 2 --passes 8",
            //    "train-sets/wsj_small.dparser.vw.gz",
            //    "train-sets/ref/search_dep_parser_arceager.stderr",
            //    "");

            //string cwd = Directory.GetCurrentDirectory();
            //RunTestsHelper.ExecuteTest(
            //    65,
            //    "-k -c -d train-sets/er_small.vw --passes 6 --search_task entity_relation --search 10 --constraints --search_alpha 1e-8",
            //    "train-sets/er_small.vw",
            //    "train-sets/ref/search_er.stderr",
            //    "");
            //RunTestsHelper.ExecuteTest(
            //    46,
            //    "-k -c -d train-sets/sequence_data --passes 20 --search_rollout ref --search_alpha 1e-8 --search_task sequence_demoldf --csoaa_ldf m --search 5 --holdout_off -f models/sequence_data.ldf.model --noconstant",
            //    "train-sets/sequence_data",
            //    "train-sets/ref/sequence_data.ldf.train.stderr",
            //    "");
            //ExecuteTest(
            //    1,
            //    "-k -l 20 --initial_t 128000 --power_t 1 -d /s/vw_rajan/test/train-sets/0001.dat -f /s/vw_rajan/test/models/0001_1.model -c --passes 8 --invariant --ngram 3 --skips 1 --holdout_off",
            //    "/s/vw_rajan/test/train-sets/0001.dat",
            //    "/s/vw_rajan/test/train-sets/ref/0001.stderr",
            //    "");

            //ExecuteTest(
            //    130,
            //    "--cb_explore_adf --bag 3 -d /s/vw_rajan/test/train-sets/cb_test.ldf --noconstant -p cbe_adf_bag.predict",
            //    "/s/vw_rajan/test/train-sets/cb_test.ldf",
            //    "/s/vw_rajan/test/train-sets/ref/cbe_adf_bag.stderr",
            //    "/s/vw_rajan/test/pred-sets/ref/cbe_adf_bag.predict");

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

            uint snum = VowpalWabbitInterface.HashSpace("s");
            featureSpace[0].name = (byte)'s';
            sfeatures[0].weight_index = VowpalWabbitInterface.HashFeature("p^the_man", snum);
            sfeatures[0].x = 1;
            // add the character "delta" to test unicode
            // do it as a string to test the marshaling is doing pinning correctly.
            const string s = "w^thew^man\u0394";
            sfeatures[1].weight_index = VowpalWabbitInterface.HashFeature(s, snum);
            sfeatures[1].x = 1;
            sfeatures[2].weight_index = VowpalWabbitInterface.HashFeature("w^man", snum);
            sfeatures[2].x = 1;
            featureSpace[0].len = 3;

            uint tnum = VowpalWabbitInterface.HashSpace("t");
            featureSpace[1].name = (byte)'t';
            tfeatures[0].weight_index = VowpalWabbitInterface.HashFeature("p^un_homme", tnum);
            tfeatures[0].x = 1;
            tfeatures[1].weight_index = VowpalWabbitInterface.HashFeature("w^un", tnum);
            tfeatures[1].x = 1;
            tfeatures[2].weight_index = VowpalWabbitInterface.HashFeature("w^homme", tnum);
            tfeatures[2].x = 1;
            featureSpace[1].len = 3;

            IntPtr importedExample = VowpalWabbitInterface.ImportExample(vw, featureSpacePtr, (IntPtr)featureSpace.Length);

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
            IntPtr vw = VowpalWabbitInterface.Initialize(string.Join(' ', "-q st -d", Path.Join(new [] {"..", "..", "..", "test", "train-sets", "0002.dat"}), "-f out"));

            VowpalWabbitInterface.StartParser(vw, false);

            int count = 0;
            IntPtr example = IntPtr.Zero;
            while (IntPtr.Zero != (example = VowpalWabbitInterface.GetExample(vw)))
            {
                float label = VowpalWabbitInterface.GetLabel(example);

                count++;
                IntPtr featureSpaceLen = (IntPtr)0;
                IntPtr featureSpacePtr = VowpalWabbitInterface.ExportExample(vw, example, ref featureSpaceLen);

                VowpalWabbitInterface.FEATURE_SPACE[] featureSpace = new VowpalWabbitInterface.FEATURE_SPACE[(int)featureSpaceLen];
                int featureSpace_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE_SPACE));

                for (int i = 0; i < (int)featureSpaceLen; i++)
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
            IntPtr vw = VowpalWabbitInterface.Initialize(string.Join(' ', "-d", Path.Join(new [] {"..", "..", "..", "test", "train-sets", "0002.dat"}), "-f out"));

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

        private static void RunFlatExampleTestEx()
        {
            //IntPtr vw = VowpalWabbitInterface.Initialize("-q st -d rcv1.train.raw.txt -f out");
            IntPtr vw = VowpalWabbitInterface.Initialize(string.Join(' ', "-q st -d", Path.Join(new [] {"..", "..", "..", "test", "train-sets", "0002.dat"}), "-f out"));

            VowpalWabbitInterface.StartParser(vw, false);

            uint stride = (uint)VowpalWabbitInterface.Get_Stride(vw);

            int count = 0;
            IntPtr example = IntPtr.Zero;
            while (IntPtr.Zero != (example = VowpalWabbitInterface.GetExample(vw)))
            {
                count++;

                float prediction = VowpalWabbitInterface.GetPrediction(example);
                float importance = VowpalWabbitInterface.GetImportance(example);
                float initial = VowpalWabbitInterface.GetInitial(example);
                float label = VowpalWabbitInterface.GetLabel(example);

                UInt32 tag_len = (UInt32)VowpalWabbitInterface.GetTagLength(example);
                byte[] tag = new byte[tag_len];
                if (tag_len > 0)
                    Marshal.Copy(VowpalWabbitInterface.GetTag(example), tag, 0, (int)tag_len);

                UInt32 num_features = (UInt32)VowpalWabbitInterface.GetFeatureNumber(example);
                VowpalWabbitInterface.FEATURE[] f;
                if (num_features > 0)
                {
                    f = new VowpalWabbitInterface.FEATURE[num_features];

                    IntPtr feature_count = (IntPtr)0;
                    IntPtr ret = VowpalWabbitInterface.GetFeatures(vw, example, ref feature_count);

                    int feature_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE));
                    for (int i = 0; i < (int)feature_count; i++)
                    {
                        IntPtr curfeaturePos = new IntPtr(ret.ToInt32() + i * feature_size);
                        f[i] = (VowpalWabbitInterface.FEATURE)Marshal.PtrToStructure(curfeaturePos, typeof(VowpalWabbitInterface.FEATURE));
                    }
                }

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

                IntPtr featureSpaceLen = (IntPtr)0;
                IntPtr featureSpacePtr = VowpalWabbitInterface.ExportExample(vw, ex, ref featureSpaceLen);

                this.featureSpace = new VowpalWabbitInterface.FEATURE_SPACE[(int)featureSpaceLen];
                int featureSpace_size = Marshal.SizeOf(typeof(VowpalWabbitInterface.FEATURE_SPACE));

                for (int i = 0; i < (int)featureSpaceLen; i++)
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

                VowpalWabbitInterface.ReleaseFeatureSpace(featureSpacePtr, (IntPtr)featureSpaceLen);
            }
        }

        private static void RunLDAPredict()
        {
            IntPtr vw = VowpalWabbitInterface.Initialize("-i wiki1k.model -t --quiet");

            IntPtr example = VowpalWabbitInterface.ReadExample(vw, "| 0:1 2049:6 2:3 5592:1 2796:1 6151:1 6154:1 6157:2 6160:2 1027:2 6168:1 4121:1 6170:1 4124:1 29:1 35:1 2088:1 2091:1 2093:2 2095:3 4145:3 5811:1 53:1 58:1 6204:6 66:2 69:2 4167:1 6216:2 75:3 2402:1 86:1 2135:2 3126:1 4185:1 90:4 2144:1 4193:1 99:1 7185:2 2156:1 110:2 2161:1 114:2 1043:1 2165:1 2166:3 119:2 6265:1 4222:3 4224:1 4230:1 705:1 2674:1 6287:1 2192:1 145:7 2198:1 2200:2 4263:1 6312:1 5148:1 4269:3 6320:4 2227:1 4283:1 4285:2 1397:2 197:2 2246:3 2247:12 201:1 4299:1 2253:1 6351:4 6353:1 4306:1 6179:1 212:1 215:3 2264:1 3108:1 2266:1 224:1 4321:1 6372:1 229:1 2281:4 6381:1 4336:1 241:2 6388:1 2294:1 2297:1 1066:1 6402:1 6405:1 6410:7 6412:2 2322:5 2329:2 282:2 6191:1 6428:1 6431:1 6433:1 4386:21 6436:5 4390:3 6439:3 296:3 1415:3 6444:3 2350:2 2354:5 307:1 6457:3 315:1 319:1 4416:4 4419:1 325:1 326:2 6472:1 6474:1 334:2 1421:2 2384:1 1516:1 340:1 4438:1 344:2 6492:5 2401:1 354:1 4452:2 6505:4 402:3 4463:1 2418:1 2451:3 375:1 4472:1 4478:2 4479:2 2437:2 4487:1 4489:2 4493:2 2448:1 5528:1 4498:1 6547:4 6549:1 406:2 2673:1 2456:2 6554:1 4507:1 4513:1 418:3 6563:1 6566:1 5873:1 2472:10 1095:1 6572:1 4525:1 4529:2 2485:2 4535:15 6587:1 444:3 6590:1 449:1 456:1 2509:6 6221:3 6562:1 2467:1 468:1 902:2 2519:1 2607:1 4653:1 6626:1 422:1 2539:6 493:4 494:1 4591:1 6644:2 3156:1 2554:1 509:1 4606:2 2562:1 516:1 2570:2 524:2 6669:1 2576:1 2577:1 4626:1 6678:1 2584:1 6916:2 538:1 7600:1 547:2 549:2 553:9 555:1 2337:1 4655:1 567:1 5679:1 570:2 6722:2 579:2 6727:2 4793:1 586:1 590:4 2643:15 4694:14 4696:6 4698:1 603:3 4700:1 6749:1 6294:1 4704:1 613:1 4710:2 2833:1 6247:1 1469:1 6769:1 6770:1 629:1 4727:1 2682:4 640:1 642:1 6793:1 2703:1 659:6 772:1 664:1 2714:1 1135:4 3525:1 4768:2 674:1 678:1 4783:1 7624:2 690:1 115:1 1481:1 697:4 6843:1 2748:1 2753:2 6262:2 6854:1 4807:1 6856:2 2763:2 6863:1 2770:1 5923:3 6869:1 4824:2 4834:2 1489:1 2793:4 4844:2 4848:2 2801:1 755:1 2807:1 763:1 2815:2 1152:1 2818:2 2820:2 7638:1 778:1 6923:1 2831:4 6929:1 4882:1 4887:2 4888:16 6940:6 798:2 6950:2 4904:2 809:1 4907:1 4909:4 2870:1 4919:3 4922:2 2879:6 4930:1 4932:5 2892:1 842:1 6988:1 846:1 4943:1 6999:3 4952:1 864:1 4966:5 1853:2 2929:1 7026:2 5267:1 4984:1 4987:1 894:1 6440:1 7042:1 7045:1 4998:3 2953:2 7050:1 2955:3 7053:2 5014:1 836:1 5018:1 3443:2 924:4 7071:8 7072:1 930:1 936:3 5033:3 5036:1 942:2 2991:1 5047:1 7096:1 7099:2 3005:1 3006:3 3008:1 962:3 963:1 3013:1 967:2 5065:3 2419:1 5068:1 5070:1 976:1 977:1 7125:1 3031:1 7130:24 3039:3 7137:2 5090:1 5091:2 996:17 997:3 3047:2 7147:1 7149:1 5105:1 3060:1 3062:13 7159:1 5112:1 3066:4 5631:1 1022:1 1023:1 7171:1 5126:4 1032:1 5131:4 3087:1 2904:1 3090:1 7187:3 5147:1 3100:1 7200:2 7201:4 1058:2 7203:5 5156:2 7207:2 1065:6 5162:3 3116:6 5165:1 7214:1 3119:1 7222:1 5180:1 3133:1 1086:2 5183:15 7233:1 5188:2 7239:4 5192:1 1097:1 5194:2 405:1 4621:1 5200:1 3153:1 855:1 7252:2 1112:1 5211:7 7675:2 7264:1 5218:2 2235:1 5220:1 3173:1 1129:2 1130:5 3181:1 1134:1 7279:1 3184:1 3186:1 1139:1 191:1 3197:1 5248:2 5249:1 993:1 2582:1 1160:2 1165:1 7315:2 3223:1 7321:1 3229:2 4293:1 2631:1 7334:7 3239:3 7338:3 3243:1 5293:2 7344:1 7348:1 6345:3 1226:1 1216:2 3041:1 2361:1 3445:1 3273:1 7370:2 3277:1 3280:4 7378:1 7381:1 3287:4 3288:1 3295:2 6520:1 5348:1 5349:5 7398:1 3303:1 5354:1 5357:1 5358:2 7408:1 5365:2 4991:1 5372:2 7421:1 5374:8 5376:1 1921:1 7434:1 3342:1 1295:1 1296:1 3349:3 6361:1 1306:2 1583:1 5409:3 6113:1 2950:1 3975:1 5420:11 7469:1 1928:1 3381:2 1334:1 5001:5 5434:1 7391:2 1341:1 7487:1 1345:2 7491:1 5449:1 1355:1 2957:1 7505:2 5458:6 3114:1 5460:2 3641:2 7512:1 5466:1 5470:1 5350:1 7526:1 7529:1 7531:1 1388:2 5488:1 1395:3 7541:2 7546:1 1258:1 1407:1 3456:2 7555:2 7557:1 7558:1 5511:2 7560:1 7563:1 4674:1 1424:2 7576:4 3483:3 1437:2 5535:3 7584:1 5539:1 1449:1 5231:1 5548:1 5549:5 3503:1 5552:1 1458:1 5556:1 7611:1 3517:2 3317:3 5570:2 1477:6 5576:2 5577:1 3530:1 3531:1 1485:1 5585:1 7210:1 1492:1 5590:2 5591:1 3544:1 118:1 1502:1 3551:1 3558:3 1513:1 5612:1 3565:2 6397:1 5616:1 4691:2 5622:7 7671:1 3577:1 5626:1 6393:1 1532:2 5629:1 3583:2 7683:2 3590:3 7689:1 5644:1 5650:12 7699:1 5654:3 5655:1 3616:1 1569:1 1572:1 4485:3 5678:4 3631:16 5683:1 5686:1 5687:1 5688:2 5689:5 3646:4 3648:3 1608:15 951:1 5718:2 1625:2 3692:2 274:1 1646:4 3695:1 5751:1 5762:2 3727:3 3737:1 1690:3 5787:1 5794:1 3747:3 5799:4 5805:1 5808:5 3763:4 1716:2 287:1 1725:1 5825:1 7559:1 7457:4 3785:2 5834:1 1746:1 3795:1 1751:15 5859:1 1764:6 5863:1 4392:1 1789:1 5896:1 3860:3 1813:5 5912:1 1822:5 1826:1 3875:6 1828:1 3879:3 3880:1 353:2 3885:6 5934:1 3890:1 6451:2 5946:8 5947:1 3901:3 2653:3 3905:2 5955:2 3908:2 1861:1 1862:1 5959:1 1494:1 5431:1 7139:4 3925:4 5974:1 5975:1 3931:1 1884:3 881:1 1888:1 4411:1 3944:2 3948:1 3949:1 3951:2 3956:5 1910:1 3961:1 6010:1 1918:2 6016:1 320:4 5441:1 3976:1 6027:2 3985:1 1947:1 6045:3 4001:1 6811:1 4009:4 1965:1 1966:1 1967:1 328:1 6131:1 4085:2 1985:1 6083:1 4036:1 4039:1 6135:1 1996:3 6093:1 1999:1 1016:1 4054:5 4055:1 4060:1 2016:2 4432:1 4073:1 2028:5 2035:1 6133:1 2039:5 4436:1");
            float score = VowpalWabbitInterface.Learn(vw, example);

            for (int i = 0; i < 10; i++)
            {
                float topicPrediction = VowpalWabbitInterface.GetTopicPrediction(example, (IntPtr)i);
                Console.Write("{0} ", topicPrediction);
            }
            Console.Write("\n");

            VowpalWabbitInterface.FinishExample(vw, example);
        }

        private static void RunVWParse_and_VWLearn()
        {
            // parse and cache
            IntPtr vw0 = VowpalWabbitInterface.Initialize(@"-d 0002.dat -c");
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
            IntPtr vw = VowpalWabbitInterface.Initialize(@"--quiet --random_seed 276518665 -f save_file.reg --readable_model reable.reg");
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

                IntPtr importedExample = VowpalWabbitInterface.ImportExample(vw, featureSpacePtr, (IntPtr)vwInstanceEx.featureSpace.Length);
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

