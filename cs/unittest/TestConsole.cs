using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VW;

namespace cs_unittest
{
    [TestClass]
    public class TestConsoleClass : TestBase
    {
        [TestMethod]
        [TestCategory("Vowpal Wabbit")]
        public void TestConsole()
        {
            var arrayModelPath = Path.GetTempFileName();
            var newlineModelPath = Path.GetTempFileName();
            var nativeModelPath = Path.GetTempFileName();

            // Note: deployment item is not working on build server
            cs_vw.Program.Main(new[] { @"..\cs\unittest\json\test_array.json", "-f", arrayModelPath });
            cs_vw.Program.Main(new[] { @"..\cs\unittest\json\test_newline.json", "-f", newlineModelPath });

            // compare model
            using (var vw = new VowpalWabbit("-f " + nativeModelPath))
            {
                vw.Learn("1 | f:1");
                vw.Learn("0 | f:2");
            }

            var arrayModel = File.ReadAllBytes(arrayModelPath);
            var newlineModel = File.ReadAllBytes(newlineModelPath);
            var nativeModel = File.ReadAllBytes(nativeModelPath);

            CollectionAssert.AreEqual(nativeModel, arrayModel);
            CollectionAssert.AreEqual(newlineModel, arrayModel);
        }
    }
}
