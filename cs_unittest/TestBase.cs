using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    [TestClass]
    public abstract class TestBase
    {
        [TestInitialize]
        public void InitializeTest()
        {
            if (Directory.Exists("models"))
            {
                Directory.Delete("models", true);
            }

            Directory.CreateDirectory("models");
        }
    }
}
