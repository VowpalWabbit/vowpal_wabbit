using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Linq;
using VW;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace cs_unittest
{
    [TestClass]
    public abstract class TestBase : IDisposable
    {
        public TestBase()
        {
            this.Init();
        }

        [TestInitialize]
        public void Init()
        {
            // CMake will copy test files to a "test" sub-directory of the binary output directory
            var basePath = Path.GetDirectoryName(typeof(TestBase).Assembly.Location);
#if NETCOREAPP3_0_OR_GREATER
            var testPath = Path.Join(basePath, "test");
#else
            var testPath = basePath + @"\test";
#endif
            Debug.Assert(Directory.Exists(testPath), $"Could not find directory: {testPath}");

            Environment.CurrentDirectory = testPath;

            if (!Directory.Exists("models"))
            {
                Directory.CreateDirectory("models");
            }
        }

        public void Dispose()
        {
            try
            {
                if (Directory.Exists("models"))
                {
                    Directory.Delete("models", true);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to delete model directory: "+ex.Message);
            }
        }
    }
}
