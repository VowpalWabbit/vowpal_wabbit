using cs_testcommon;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using VW;

namespace cs_leaktest
{
    public class TestWrappedBase
    {
        [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Ansi)]
        static extern IntPtr LoadLibrary([MarshalAs(UnmanagedType.LPStr)]string lpFileName);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool FreeLibrary(IntPtr hModule);

        /// <summary>
        /// Implement custom StackTrace so one can easily click in Test Explorer
        /// </summary>
        internal class CustomException : Exception
        {
            private readonly string stackTrace;

            internal CustomException(string message, string stackTrace) : base(message)
            {
                this.stackTrace = stackTrace;
            }

            public override string StackTrace
            {
                get
                {
                    return this.stackTrace;
                }
            }
        }
        public TestContext TestContext { get; set; }

        protected void Run(string type, string method)
        {
            using (var vld = new VisualLeakDetector())
            {
                try
                {
                    var basePath = Path.GetDirectoryName(typeof(VisualLeakDetector).Assembly.Location);

                    var handle = LoadLibrary(basePath + @"\\VowpalWabbitCore.dll");
                    var appDomain = AppDomain.CreateDomain("Test1");

                    try
                    {
                        ITestRunner test1 = (ITestRunner)appDomain.CreateInstanceFromAndUnwrap(basePath + @"\\cs_unittest.dll", "cs_unittest.TestRunner");

                        Environment.CurrentDirectory = TestContext.TestDir + @"\..\..\..\test";

                        var result = test1.Run(type, method);

                        if (result != null)
                        {
                            // check for exception marker
                            var index = result.IndexOf("#-#-#-#-#-#-#");

                            if (index == -1)
                            {
                                Assert.Fail(result);
                            }

                            throw new CustomException(result.Substring(0, index), result.Substring(index + 13));
                        }
                    }
                    finally
                    {
                        AppDomain.Unload(appDomain);
                    }

                    FreeLibrary(handle);
                    FreeLibrary(handle);

                    Debug.WriteLine("vld.ReportLeaks.1");

                    vld.ReportLeaks();

                    Debug.WriteLine("vld.ReportLeaks.2");
                    var message = string.Concat(vld.Messages.Select(t => t.Item2));

                    var blocks = message.Split(new[] { "---------- Block " }, StringSplitOptions.None)
                      .Where(block => Regex.IsMatch(block, "^\\d+ at"))
                      .ToList();

                    Assert.AreEqual(0, blocks.Count, string.Join("\n", blocks));

                    LoadLibrary(@"C:\work\vw2\cs_unittest\bin\x64\Debug\VowpalWabbitCore.dll");
                }
                finally
                {
                    vld.MarkAllLeaksAsReported();
                }
            }
        }
    }
}
