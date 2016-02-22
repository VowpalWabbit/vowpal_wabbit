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
    public class TestConsoleClass
    {
        [TestMethod]
        [DeploymentItem("json/test_array.json")]
        [DeploymentItem("json/test_newline.json")]
        public void TestConsole()
        {
            cs_vw.Program.Main(new[]{"test_array.json","-f","array.model"});
            cs_vw.Program.Main(new[]{"test_newline.json","-f","newline.model"});

            // compare model
            using (var vw = new VowpalWabbit("-f native.model"))
            {
                vw.Learn("1 | f:1");
                vw.Learn("0 | f:2");
            }

            var arrayModel = File.ReadAllBytes("array.model");
            var newlineModel = File.ReadAllBytes("newline.model");
            var nativeModel = File.ReadAllBytes("native.model");

            CollectionAssert.AreEqual(nativeModel, arrayModel);
            CollectionAssert.AreEqual(newlineModel, arrayModel);
        }
    }
}
