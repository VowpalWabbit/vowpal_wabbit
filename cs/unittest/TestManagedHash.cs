using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Diagnostics;
using System.Text;
using TrainSet0002Dat;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class TestManagedHash
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
            "934625             ",
            "123"
        };

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestHash()
        {
            InternalTestHash("");
            InternalTestHash("--hash all");
            InternalTestHash("--hash strings");
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestHashSpace()
        {
            using (var vw = new VowpalWabbit(""))
            {
                Assert.AreEqual(0ul, vw.HashSpace(" "));
                Assert.AreEqual(0ul, vw.HashSpace("0"));
            }
        }

        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestHashUnicodeSpace()
        {
            using (var vw = new VowpalWabbit(""))
            {
                var value = "ArticleTitleThe_25_Biggest_Art_Moments_of_2012" + (char)160;
                // Encoding.UTF8.GetMaxByteCount
                var nativeHash = vw.HashSpaceNative(value);
                var managedHash = vw.HashSpace(value);

                Assert.AreEqual(nativeHash, managedHash);
            }
        }

        private void InternalTestHash(string args)
        {
            var stopWatchNative = new Stopwatch();
            var stopWatchManaged = new Stopwatch();

            using (var vw = new VowpalWabbit(args))
            {
                for (int i = 0; i < 10000; i++)
                {
                    foreach (var item in data)
                    {
                        stopWatchNative.Start();
                        var nativeHash = vw.HashSpaceNative(item);
                        stopWatchNative.Stop();

                        stopWatchManaged.Start();
                        var managedHash = vw.HashSpace(item);
                        stopWatchManaged.Stop();

                        Assert.AreEqual(nativeHash, managedHash, item);
                    }
                }
            }

            Console.WriteLine("Args: " + args);
            Console.WriteLine("native:  {0}", stopWatchNative.Elapsed);
            Console.WriteLine("managed: {0}", stopWatchManaged.Elapsed);
        }
    }
}
