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
  public abstract class TestBase : IDisposable
  {
    public TestBase()
    {
      Directory.CreateDirectory("models");
    }

        public void Dispose()
        {
            if (Directory.Exists("models"))
            {
                Directory.Delete("models", true);
            }
        }
    }
}
