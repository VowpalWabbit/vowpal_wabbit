using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MultiWorldTesting;

namespace ExploreSample
{
    class Program
    {
        public static void Main(string[] args)
        {
            cs_test.ExploreOnlySample.Run();
        }
    }

    public class HashBenchmarks
    {
        static string[] data = 
        {
            "wLceNLHVtpuZYtPwPaQ",
            "949007             ",
            "Kvq7Hj6RSImhZUhcZuL",
            "KDqmcZO2h6CIM1j    ",
            "656656             ",
            "X8bRcLpb8yrIkA2A   ",
            "k5USpack9N         ",
            "Okv90y9lOamog3qXRIk",
            "860628             ",
            "903251             ",
            "289149             ",
            "727293             ",
            "660473             ",
            "ciajGNL930GlKi5b   ",
            "744202             ",
            "458807             ",
            "466125             ",
            "101824             ",
            "0RVP8HWyKGFjkJG8RA ",
            "mm3XQ0ZKJQ4rPmtNN  ",
            "ZL7Z6bzFVsL0VQGe5ss",
            "690592             ",
            "Apbr4WNUDHmL7OWxm  ",
            "342052             ",
            "286245             ",
            "JbIIXVVbS3Y79uj4iI ",
            "S9E90IvFAVt        ",
            "z2QWOpzi63         ",
            "gnCClcujq79        ",
            "hOZPaw9s4922I3S    ",
            "mFWZjVtCOiymM2     ",
            "m6a93w7IRLNaadJbL  ",
            "758870             ",
            "164290             ",
            "971935             ",
            "MVObGSH9iWxiyvp    ",
            "135400             ",
            "T2b9WalhX9c        ",
            "CQaS6KtGArRLtM5v   ",
            "B0lNkkeP57ZLJjZAwfP",
            "695049             ",
            "BSbUX2YPm1daHvo6   ",
            "ReVgoh7mtQpghPDl   ",
            "I0RnHRdk5IRFHJZaZST",
            "489901             ",
            "a9IZGkY6WLtX0X37D  ",
            "061731             ",
            "402102             ",
            "IgRGpl2Z0OdgNzr6AH ",
            "tNlzNvlPQ0hXFlzjpe8",
            "m2JmhQ8L6DEnauuvSst",
            "141010             ",
            "534087             ",
            "599686             ",
            "000093             ",
            "707313             ",
            "563622             ",
            "HlcM6fNDjW         ",
            "4qEn6lfmhd2b6Fo    ",
            "ph5x9nJTFV         ",
            "783062             ",
            "403127             ",
            "fGbvUKatET3SAf0rfA ",
            "IduDv41Z1z7Opirz   ",
            "625285             ",
            "HbsPUqTZvWHI4ylB   ",
            "554240             ",
            "849636             ",
            "1ElP3So1fCS        ",
            "539836             ",
            "jELB4FrYkqwpmecr   ",
            "Ko4EWBb3gFqN0PR7pvf",
            "VMX4dVyfAZ0V9VwK   ",
            "K0BYm86Zg8PogMNSo  ",
            "ajfcoff0sqt        ",
            "373791             ",
            "220160             ",
            "dxZoyNeZZMiO       ",
            "286375             ",
            "DEy4nNiHHd9nN      ",
            "3gverMSb6ANY3wLj   ",
            "ATGPA40OShUer      ",
            "548754             ",
            "7NdgIl223apO       ",
            "aaas hu as 撒 asfasd	阿萨",
            "oof Ồ hử hị ộ ở ỗ õ ẽ uyễn \r \t \n \\    ",
            "934625             "
        };

        public static void Run()
        {
            var sw = new System.Diagnostics.Stopwatch();
            foreach (string item in data)
            {
                ulong ring = MurMurHash3.ComputeIdHash(item);
                ulong gbs = MurMurHash3.ComputeIdHashGetBytesStruct(item);
                ulong gb = MurMurHash3.ComputeIdHashGetBytes(item);
                ulong mgb = MurMurHash3.ComputeIdHashManualGetBytes(item);

                if (ring != gbs || ring != gb || ring != mgb)
                {
                    throw new Exception();
                }
            }
            Console.WriteLine("Correct.");

            sw.Restart();
            for (int i = 0; i < 1000000; i++)
            {
                foreach (string item in data)
                {
                    MurMurHash3.ComputeIdHash(item);
                }
            }

            Console.WriteLine("Ring: " + sw.ElapsedMilliseconds);

            sw.Restart();
            for (int i = 0; i < 1000000; i++)
            {
                foreach (string item in data)
                {
                    MurMurHash3.ComputeIdHashGetBytesStruct(item);
                }
            }

            Console.WriteLine("Struct Get Bytes: " + sw.ElapsedMilliseconds);

            sw.Restart();
            for (int i = 0; i < 1000000; i++)
            {
                foreach (string item in data)
                {
                    MurMurHash3.ComputeIdHashGetBytes(item);
                }
            }
            Console.WriteLine("Get Bytes: " + sw.ElapsedMilliseconds);

            sw.Restart();
            for (int i = 0; i < 1000000; i++)
            {
                foreach (string item in data)
                {
                    MurMurHash3.ComputeIdHashManualGetBytes(item);
                }
            }
            Console.WriteLine("Manual Get Bytes: " + sw.ElapsedMilliseconds);
        }
    }
}
