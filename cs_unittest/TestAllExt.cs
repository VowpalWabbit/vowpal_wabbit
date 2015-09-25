using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_unittest
{
    public partial class TestAll
    {
        [TestInitialize]
        public void TestInit()
        {
            Directory.CreateDirectory("models");
        }
    }
}
