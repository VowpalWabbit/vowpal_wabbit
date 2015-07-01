using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;

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
